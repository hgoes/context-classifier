import numpy as np
from math import exp
from ConfigParser import ConfigParser

class ClassifierSet:
    def __init__(self,classifiers):
        self.classifiers = [ c for c in classifiers ]
    def step(self,v):
        pos = 0
        rst = None
        prob = 0
        for classifier in self.classifiers:
            (st,max,cprob) = classifier.evaluate(v)
            #if cprob > 0:
            #    prob = cprob
            #    rst = st
                #print "Classifier",classifier.name,"matched with state",st,"and value",prob
            #    break
            #print "Classifier",classifier.name,"didn't match"
            pos += 1
        if rst is not None:
            if pos!=0:
                cl = self.classifiers.pop(pos)
                self.classifiers = [cl]+self.classifiers
        return (rst,prob)

def fuzzy(avg,var):
    return lambda x: max(0,1 - abs(avg-x)/var)

class Classifier:
    def __init__(self,ruleset,memb=[],name=""):
        self.ruleset = ruleset
        self.membership = memb
        self.name = name
    def evaluate(self,v):
        g = self.ruleset.evaluate(v)
        #print "Classifier ",self.name,"produced value",g
        max = 0
        max_el = None
        for (k,f) in self.membership:
            r = f(g)
            if max < r:
                max = r
                max_el = k
        return (max_el,max,g)

class RuleSet:
    def __init__(self,rules):
        self.rules = rules
    def evaluate(self,v):
        r = 0.0
        d = 0.0
        for rule in self.rules:
            w = rule.weight(v)
            e = rule.evaluate(v)
            #print "Weight",w
            #print "Res",e
            r += w*e
            d += w
        return r/d
    def __str__(self):
        res = "RuleSet: | "
        for rule in self.rules:
            res += str(rule)+" |"
        return res

def bit(str):
    if str == "0":
        return 1
    else:
        return 0

def parse_rules(f):
    cfgparser = ConfigParser()
    cfgparser.readfp(open(f))
    dims=cfgparser.getint("DEFAULT","dimensions")
    rules = []
    for rule in cfgparser.sections():
        means = np.array(map(float,cfgparser.get(rule,"mean").split()))
        var = np.array(map(float,cfgparser.get(rule,"sigma").split())).reshape((dims,dims))
        cons = map(float,cfgparser.get(rule,"consequence").split())
        if cfgparser.has_option(rule,"bitvec"):
            bitvec = np.array(map(bit,cfgparser.get(rule,"bitvec").split())).nonzero()
        else:
            bitvec = None
        bitvec = None
        result = np.array(cons[0:-1])
        roff = cons[-1]
        rules.append(ComplexRule(result,roff,means,var,bitvec))
    return RuleSet(rules)

class Rule:
    def __init__(self,rvec,roff,bitvec=None):
        self.rvec = np.asmatrix(rvec)
        self.roff = roff
        self.bitvec = bitvec
    def evaluate(self,v):
        return (self.rvec*(np.asmatrix(v).T))[0,0] + self.roff
    def weight(self,v):
        t = v - self.means()
        if self.bitvec is not None:
            t.put(self.bitvec,0)
        return exp(-0.5*(t*self.variance()*t.T)[0,0])
    def means(self):
        abstract
    def variance(self):
        abstract

class SimpleRule(Rule):
    def __init__(self,rvec,roff,rules,bitvec=None):
        Rule.__init__(self,rvec,roff,bitvec)
        self.rules = rules
    def means(self):
        return np.asmatrix([m for (m,v) in self.rules])
    def variance(self):
        return np.asmatrix(np.diag([1/v for (m,v) in self.rules]))

class ComplexRule(Rule):
    def __init__(self,rvec,roff,vmean,covar,bitvec=None):
        Rule.__init__(self,rvec,roff,bitvec)
        self.vmean = np.asmatrix(vmean)
        self.covar = np.asmatrix(covar)
    def means(self):
        return self.vmean
    def variance(self):
        return self.covar
    def __str__(self):
        return "ComplexRule:\nCovar: "+str(self.covar)+"\nMeans: "+str(self.vmean)+"\nResult: "+str(self.rvec)
