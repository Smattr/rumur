#
# Regular cron jobs for the rumur package
#
0 4	* * *	root	[ -x /usr/bin/rumur_maintenance ] && /usr/bin/rumur_maintenance
