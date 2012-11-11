To test process, from a terminal logged as root:

#cp testproc /usr/sbin
#cp testprocsh /etc/init.d/
#chmod 775 /usr/sbin/testproc
#chown root:root /usr/sbin/testproc
#chmod 775 /etc/init.d/testprocsh
#chown root:root /etc/init.d/testprocsh
#/etc/init.d/testprocsh start

And check the file xaphub.pid is created on /var/run with:

#ls /var/run

Stop the process with:

#/etc/init.d/testprocsh stop

And the file xaphub.pid there isn't exists on /var/run, check:

#ls /var/run
