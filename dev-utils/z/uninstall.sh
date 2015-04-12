#!/bin/sh

sudo rm -f /etc/bash_completion.d/z_complete.sh
sudo rm -f ~/.z/z.sh
sed -i '/complete -F z_comp z/d' ~/.bashrc
sed -i '/alias z=". ~\/.z\/z.sh/d' ~/.bashrc
