Unfortunately, [my mirror](https://github.com/brianwalenz/opengridscheduler) of Open Grid Scheduler
was exhibiting a few bugs that were driving me nuts.  Namely, that load balancing by load was not
working, it was always balancing by sequence number - in other words, jobs would fill up host A,
then host B, then host C, etc, instead of running one job on host A, then host B, etc, round-robin..

I therefore took my own advice and considered 
[Son of Grid Engine](http://arc.liv.ac.uk/SGE/).

This repo is a copy of 
Version [8.1.9](http://arc.liv.ac.uk/downloads/SGE/releases/8.1.9/),
with modifications for FreeBSD.

## Compilation:

Easy, but not as eas as 'gmake', and not any easier than Open Grid Scheduler (in fact, exactly the same).

```
cd source

setenv AIMK_ARGS "-spool-classic -no-java -no-jni -no-gui-inst -no-secure -no-qmake -no-qtcsh"

./aimk $AIMK_ARGS -only-depend
./scripts/zerodepend
./aimk $AIMK_ARGS depend
./aimk $AIMK_ARGS 
./aimk $AIMK_ARGS -man

setenv SGE_ROOT  /usr/local/sge


./scripts/distinst -y -noexit -local -allall

./scripts/distinst -y -noexit -local -libs fbsd-amd64 -- \
   man common sge_qmaster sge_execd sge_shadowd sge_shepherd sge_coshepherd \
   qstat qsub qalter qconf qdel qacct qmod qsh utilbin jobs qhost qmake qmon \
   qping qloadsensor.exe sgepasswd qquota

```

## Installation:

The 'official' installation wants you to run '/usr/local/sge/install_qmaster' and '/usr/local/sge/install_execd'. I haven't done that recently, instead, just the following steps:

pw groupadd sgeadmin -g 103
pw useradd sgeadmin -u 103 -g 103 -h - -d /nonexistent -s /sbin/nologin -c "Sun Grid Engine Admin"

cp -p /usr/local/sge/rc.d_sge /usr/local/etc/rc.d/sge

#  Load SGE on boot.  Add:
#    sge_qmaster_enable="YES"
#    sge_execd_enable="YES"
vi /etc/rc.conf

#  Add '. /usr/local/sge/default/common/settings.sh' so grid jobs have SGE configured too
vi /etc/profile

#  Add 'source /usr/local/sge/default/common/settings.csh'
vi ~/.cshrc

### Addendum:

I wanted a fresh install, so I did (finally) run 'install_qmaster' to get the initial
config setup.  I didn't run 'install_execd' though, and didn't muck with the startup
scripts.

I edited '$SGE_CELL/spool/qmaster/jobseqnum' to set the job counter
to what I had before.

Surprisingly, the 'accounting' file from SGE 6.2u2_1 seems
to work, and I copied that over too.