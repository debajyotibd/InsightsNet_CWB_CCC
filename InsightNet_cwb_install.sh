cd cwb
cd install-scripts

chmod +x install-linux
cd ..

sudo ./install-scripts/install-linux
ls /usr/local/bin | grep cwb


cd cwb-perl/CWB

perl Makefile.PL --config /usr/local/bin/cwb-config
make
make test
sudo make install
