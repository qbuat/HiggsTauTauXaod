import ROOT



EOS_PATH = '/eos/atlas/user/q/qbuat'
EOS_SNIFFER = ROOT.SH.SampleHandler()
EOS_LIST = ROOT.SH.DiskListEOS(EOS_PATH, "root://eosatlas/" + EOS_PATH)
ROOT.SH.ScanDir().scan(EOS_SNIFFER, EOS_LIST)

def get_one_sample(name):
    sample = filter(lambda s: s.name() == name, EOS_SNIFFER)
    if len(sample) != 1:
        raise RuntimeError('The number of samples is wrong')
    return sample[0]

def get_sample(sample_name):
    handler = ROOT.SH.SampleHandler()
    if isinstance(sample_name, (list, tuple)):
        for n in sample_name:
            sample = get_one_sample(n)
            handler.add(sample)
    else:
        sample = get_one_sample(sample_name)
        handler.add(sample)
    return handler


def get_output_name(input_name, user_name, version):
    # mc15_13TeV.341157.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_VBFH125_tautauhh.merge.AOD.e3888_s2608_s2183_r7772_r7676
    splits = input_name.split('.')
    
    project = splits[0]
    dsid = splits[1]
    name = splits[2]
    merge = splits[3]
    aod = splits[4]
    tag = splits[5]
    
    output_name = '.'.join([
            user_name,
            'hhskim',
            project,
            dsid,
            short_name(name),
            tag,
            'v{0}'.format(version)])
    return output_name

def short_name(name):
    return name.replace('PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1', 'PoPy8')
