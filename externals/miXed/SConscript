import glob
import os
import re
Import('env prefix')

#voodoo escaping, anyone got it? !!...
os.system("cd toxy && make checkwiq && make setup.wiq")

env.Append(CPPPATH = 'shared')

miXed_shared = glob.glob('shared/common/*.c')
miXed_shared.extend(Split('shared/unstable/fringe.c shared/unstable/forky.c shared/unstable/fragile.c shared/unstable/loader.c'))
hammer_shared = glob.glob('shared/hammer/*.c')
sickle_shared = glob.glob('shared/sickle/*.c')

hammer_source = glob.glob('cyclone/hammer/*.c')
hammer_src = [miXed_shared, hammer_shared, hammer_source]
hammer = env.SharedLibrary(target = 'hammer', source = hammer_src)
env.Alias('install', env.Install(os.path.join(prefix, 'extra'), hammer))
Default(hammer)

sickle_source = glob.glob('cyclone/sickle/*.c')
sickle_src = [hammer_shared, miXed_shared, sickle_shared, sickle_source]
sickle = env.SharedLibrary(target = 'sickle', source = sickle_src)
env.Alias('install', env.Install(os.path.join(prefix, 'extra'), sickle))
Default(sickle)

for hammer_extra in hammer_source:
    if (hammer_extra != 'cyclone/hammer/hammer.c'):
        src = [hammer_extra,miXed_shared,hammer_shared]
        external = env.SharedLibrary(target = re.sub("\.c$","",os.path.basename(hammer_extra)), source = src)
        env.Alias('install', env.Install(os.path.join(prefix, 'extra'), external))
        Default(external)

for sickle_extra in sickle_source:
    if (sickle_extra != 'cyclone/sickle/sickle.c'):
        target = re.sub("\.c$","",os.path.basename(sickle_extra)) + "~"
        src = [sickle_extra,sickle_shared,miXed_shared,hammer_shared]
        external = env.SharedLibrary(target = target, source = src)
        env.Alias('install', env.Install(os.path.join(prefix, 'extra'), external))
        Default(external)
        
toxy_src = glob.glob('shared/toxy/*.c')
for toxy_extra in Split('tot plustot tow widget'):
    toxy_source = "toxy/" + toxy_extra + ".c"
    src = [toxy_src,toxy_source,'toxy/widgettype.c','toxy/widgethandlers.c',miXed_shared,hammer_shared]
    external = env.SharedLibrary(toxy_extra, src)
    env.Alias('install', env.Install(os.path.join(prefix, 'extra'), external))
    Default(external)

env.Alias('install', env.Install(os.path.join(prefix, 'extra'), glob.glob('test/toxy/*.wid')))
env.Alias('install', env.Install(os.path.join(prefix, 'doc/miXed/'), glob.glob('doc/help/*/*.pd')))
