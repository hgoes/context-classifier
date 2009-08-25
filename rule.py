import numpy as np
from ConfigParser import ConfigParser

class ClassifierSet:
    def __init__(self,classifiers):
        self.classifiers = [ c for c in classifiers ]
    def step(self,v):
        pos = 0
        rst = None
        for classifier in self.classifiers:
            (st,prob) = classifier.evaluate(v)
            if prob > 0:
                rst = st
                break
            pos += 1
        if rst is not None:
            if pos!=0:
                cl = self.classifiers[pos]
                del self.classifiers[pos]
                self.classifiers = [cl]+self.classifiers
        return rst            

def fuzzy(avg,var):
    return lambda x: max(0,1 - abs(avg-x)/var)

class Classifier:
    def __init__(self,ruleset,memb=[]):
        self.ruleset = ruleset
        self.membership = memb
    def evaluate(self,v):
        g = self.ruleset.evaluate(v)
        max = 0
        max_el = None
        for (k,f) in self.membership:
            r = f(g)
            if max < r:
                max = r
                max_el = k
        return (max_el,max)


class RuleSet:
    def __init__(self,rules):
        self.rules = rules
    def evaluate(self,v):
        r = 0.0
        d = 1.0
        for rule in self.rules:
            w = rule.weight(v)
            r += w*rule.evaluate(v)
            d *= w
        return r/d

def parse_rules(f):
    cfgparser = ConfigParser()
    cfgparser.readfp(open(f))
    dims=cfgparser.getint("DEFAULT","dimensions")
    rules = []
    for rule in cfgparser.sections():
        means = np.array(map(float,cfgparser.get(rule,"mean").split()))
        var = np.array(map(float,cfgparser.get(rule,"sigma").split())).reshape((dims,dims))
        result = np.array(map(float,cfgparser.get(rule,"consequence").split()))
        rules.append(ComplexRule(result,means,var))
    return RuleSet(rules)

class Rule:
    def __init__(self,rvec):
        self.rvec = np.asmatrix(rvec)
    def evaluate(self,v):
        return (self.rvec*(np.asmatrix(v).T))[0,0]
    def weight(self,v):
        t = v - self.means()
        return (t*self.variance()*t.T)[0,0]
    def means(self):
        abstract
    def variance(self):
        abstract

class SimpleRule(Rule):
    def __init__(self,rvec,rules):
        Rule.__init__(self,rvec)
        self.rules = rules
    def means(self):
        return np.asmatrix([m for (m,v) in self.rules])
    def variance(self):
        return np.asmatrix(np.diag([1/v for (m,v) in self.rules]))

class ComplexRule(Rule):
    def __init__(self,rvec,vmean,covar):
        Rule.__init__(self,rvec)
        self.vmean = np.asmatrix(vmean)
        self.covar = np.asmatrix(covar)
    def means(self):
        return self.vmean
    def variance(self):
        return self.covar
    def __str__(self):
        return "ComplexRule:\nCovar: "+str(self.covar)+"\nMeans: "+str(self.vmean)+"\nResult: "+str(self.rvec)

