import ROOT

def to_str_vec(l):
    if not isinstance(l, (list, tuple)):
        raise RuntimeError

    vec = ROOT.vector('string')()
    for item in l:
        vec.push_back(str(item))

    return vec
