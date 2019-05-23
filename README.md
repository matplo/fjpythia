# fjpythia
minimalistic setup for fastjet and pythia

# installation

- note: relies that ROOT (root.cern.ch) is already installed
   - setup environment for root and run `./scripts/make_root_setup.sh`

- to install fastjet 3: `./scripts/install_fastjet3.sh`
   - note, the tests on contributed algos are disabled - enable/run `make check`
- to install pythia8: `./scripts/install_pythia8.sh`
- build: `./scripts/build.sh`
- run an example: `./scripts/run.sh`
