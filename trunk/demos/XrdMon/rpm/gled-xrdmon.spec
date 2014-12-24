# revision of the original gled-grdmon package
%global rev 1

%global packagename gled-xrdmon
%global version     _VERSION_
%global release     %{rev}.%{dist}

%global prefix    /opt/gled
%global buildroot %{_tmppath}/%{packagename}-%{version}-%{release}-root-%(%{__id_u} -n)

%global service_name  gled-xrdmon
%global service_user  nobody
%global service_group nobody

# legacy support
%global __var /var

Summary:        Gled XRootD transfers monitoring
Name:           %{packagename}
Version:        %{version}
Release:        %{release}
License:        GPLv3
Group:          Applications/Monitoring
URL :           http://www.gled.org/cgi-bin/twiki/view/Main/XrdMon
Vendor:         ARDA Dashboard
BuildRoot:      %{buildroot}
BuildArch:      x86_64

# disable automatic dependency processing
# all necessary libraries are already in, should not be external requirements
AutoReqProv: no

Source0:        gled-xrdmon-%{version}-%{dist}.tar.gz
Source1:        configs.tar.gz


%description
Gled monitoring of XRootD files transfers

%prep
%setup -c -a0 -a1

# clean up SVN entries if any
find -name '.svn' | xargs rm -rf

# removing unnecessary files and directories
# The following is already called before making of the tarball
# cd gled
# ./gled/demos/XrdMon/rpm/cleanup-install-for-rpm.sh



%install
%{__rm} -rf %{buildroot}

%{__mkdir_p} %{buildroot}/%{prefix}
%{__mkdir_p} %{buildroot}/etc/gled/scripts %{buildroot}/etc/gled/functions
%{__mkdir_p} %{buildroot}/var/log/gled %{buildroot}/var/run/gled


%{__cp} -R gled/* %{buildroot}/%{prefix}

%{__install} -D -m 644 gled/demos/XrdMon/logrotate.d/gled-xrdmon %{buildroot}/etc/logrotate.d/%{service_name}

%{__cp} -R configs/* %{buildroot}/
%{__chmod} 755 %{buildroot}/etc/init.d/*


%post

if [ "$1" = "1" ]; then
# in case of first time setup
    /sbin/chkconfig --add %{service_name}
    /sbin/chkconfig %{service_name} on
# in case of update
else
    /sbin/service %{service_name} condrestart
fi

true


%preun

# stopping service before any update or removal
# if package is being updated, service will be restarted by postinstall script
/sbin/service %{service_name} stop >/dev/null 2>&1

if [ $1 = 0 ] ; then
    # before package removal
    /sbin/chkconfig --del %{service_name}
fi
                
true


%clean
%{__rm} -rf %{buildroot}


%files
%defattr(-,root,root,-)
%{prefix}/
/etc/logrotate.d/%{service_name}
/etc/init.d/%{service_name}
/etc/cron.d/*
/etc/gled

%config(noreplace) /etc/gled/collectors.cfg
%attr(640,root,root) /etc/gled/collectors.cfg


%attr(-,%{service_user},%{service_group}) /var/log/gled
%attr(-,%{service_user},%{service_group}) /var/run/gled

%doc gled/README_gled gled/demos/XrdMon/ChangeLog

%changelog gled/demos/XrdMon/ChangeLog
