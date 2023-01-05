#
# Regular cron jobs for the ostserver package
#
0 4	* * *	root	[ -x /usr/bin/ostserver_maintenance ] && /usr/bin/ostserver_maintenance
