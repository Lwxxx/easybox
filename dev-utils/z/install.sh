#!/bin/sh

sudo cp ./z_complete.sh /etc/bash_completion.d/
mkdir -p ~/.z
sudo cp ./z.sh ~/.z
if [ ! -f ~/.z/path.db ]; then
	cp ./path.db ~/.z/path.db
fi
cp ~/.bashrc ~/.bashrc.bkp
sed -i '/complete -F z_comp z/d' ~/.bashrc
sed -i '/alias z=". ~\/.z\/z.sh/d' ~/.bashrc
echo 'complete -F z_comp z' >> ~/.bashrc
echo 'alias z=". ~/.z/z.sh"' >> ~/.bashrc
