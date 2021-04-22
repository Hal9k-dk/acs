#!/bin/bash
/usr/bin/ruby /home/hal9k/acs/ui/init.rb | tee /var/log/acs/ui.log
/usr/bin/ruby /home/hal9k/acs/ui/ui.rb | tee -a /var/log/acs/ui.log
