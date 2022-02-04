import os
import sys
import struct

if sys.version_info < (2, 7):
    sys.stderr.write('You must have at least Python 2.7 for pytcpb to work '
                     'correctly.\n')
    sys.exit(1)

try:
    if '--no-setuptools' in sys.argv:
        sys.argv.remove('--no-setuptools')
        raise ImportError() # Don't import setuptools...
    from setuptools import setup, Extension
except ImportError:
    from distutils.core import setup, Extension

from distutils.command.clean import clean as Clean

class CleanCommand(Clean):
    """python setup.py clean
    """
    # lightly adapted from scikit-learn package
    description = "Remove build artifacts from the source tree"

    def _clean(self, folder):
        for dirpath, dirnames, filenames in os.walk(folder):
            for filename in filenames:
                if (filename.endswith('.so') or filename.endswith('.pyd')
                        or filename.endswith('.dll')
                        or filename.endswith('.pyc')):
                    os.unlink(os.path.join(dirpath, filename))
            for dirname in dirnames:
                if dirname == '__pycache__':
                    shutil.rmtree(os.path.join(dirpath, dirname))

    def run(self):
        Clean.run(self)
        if os.path.exists('build'):
            shutil.rmtree('build')
        self._clean('./')


is_pypy = '__pypy__' in sys.builtin_module_names

# packages to be installed
packages = ['pytcpb']

# extensions
extensions = []

if __name__ == '__main__':

    import shutil

    # See if we have the Python development headers.  If not, don't build the
    # optimized prmtop parser extension
    from distutils import sysconfig
    if not is_pypy and not os.path.exists(
            os.path.join(sysconfig.get_config_vars()['INCLUDEPY'], 'Python.h')):
        extensions = []

    # Delete old versions with old names of scripts and packages
    def deldir(folder):
        try:
            shutil.rmtree(folder)
        except OSError:
            sys.stderr.write(
                    'Could not remove old package %s; you should make sure\n'
                      'this is completely removed in order to make sure you\n'
                      'do not accidentally use the old version of pytcpb\n' %
                      folder
            )
    def delfile(file):
        try:
            os.unlink(file)
        except OSError:
            sys.stderr.write(
                    'Could not remove old script %s; you should make sure\n'
                      'this is completely removed in order to make sure you\n'
                      'do not accidentally use the old version of pytcpb\n' %
                      file
            )

    for folder in sys.path:
        folder = os.path.realpath(os.path.abspath(folder))
        if folder == os.path.realpath(os.path.abspath('.')): continue
        pytcpbf = os.path.join(folder, 'pytcpb')
        #pytcpb = os.path.join(folder, 'pytcpb.py')
        if os.path.isdir(pytcpbf): deldir(pytcpbf)
        #if os.path.exists(pytcpb): delfile(pytcpb)

    #for folder in os.getenv('PATH').split(os.pathsep):
    #    pytcpb = os.path.join(folder, 'pytcpb.py')
    #    if os.path.exists(pytcpb): delfile(pytcpb)

    cmdclass = dict(clean=CleanCommand)
    #cmdclass.update(versioneer.get_cmdclass())
    setup(name='pyTCPB',
          version="1.0.0",
          description='Wrapper to use TCPB-cpp functions from Python',
          author='Vinicius Wilian D. Cruzeiro',
          author_email='vwcruz@stanford.edu',
          license='LGPL',
          packages=packages,
          ext_modules=extensions,
          cmdclass=cmdclass
    )
