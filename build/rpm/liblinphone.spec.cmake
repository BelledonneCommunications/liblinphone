# -*- rpm-spec -*-

%define _prefix    @CMAKE_INSTALL_PREFIX@
%define pkg_prefix @BC_PACKAGE_NAME_PREFIX@

# re-define some directories for older RPMBuild versions which don't. This messes up the doc/ dir
# taken from https://fedoraproject.org/wiki/Packaging:RPMMacros?rd=Packaging/RPMMacros
%define _datarootdir       %{_prefix}/share
%define _datadir           %{_datarootdir}
%define _docdir            %{_datadir}/doc

%define build_number @PROJECT_VERSION_BUILD@
%if %{build_number}
%define build_number_ext -%{build_number}
%endif


Name:           @CPACK_PACKAGE_NAME@
Version:        @PROJECT_VERSION@
Release:        %{build_number}%{?dist}
Summary:        Phone anywhere in the whole world by using the Internet

Group:          Applications/Communications
License:        GPL
URL:            http://www.linphone.org
Source0:        %{name}-%{version}%{?build_number_ext}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-buildroot

Requires:	%{pkg_prefix}bctoolbox
Requires:	%{pkg_prefix}ortp
Requires:	%{pkg_prefix}mediastreamer
Requires:	%{pkg_prefix}belle-sip
Requires:	%{pkg_prefix}belr
%if @ENABLE_VCARD@
Requires:	%{pkg_prefix}belcard
%endif

%description
liblinphone is the voip sdk used by Linphone


%package devel
Summary:       Development libraries for liblinphone
Group:         Development/Libraries
Requires:      %{name} = %{version}-%{release}

%description    devel
This package contains header files and development libraries needed to
develop programs using the liblinphone library.

%if 0%{?rhel} && 0%{?rhel} <= 7
%global cmake_name cmake3
%define ctest_name ctest3
%else
%global cmake_name cmake
%define ctest_name ctest
%endif

# This is for debian builds where debug_package has to be manually specified, whereas in centos it does not
%define custom_debug_package %{!?_enable_debug_packages:%debug_package}%{?_enable_debug_package:%{nil}}
%custom_debug_package

%prep
%setup -n %{name}-%{version}%{?build_number_ext}

%build
%{expand:%%%cmake_name} . -DCMAKE_BUILD_TYPE=@CMAKE_BUILD_TYPE@ -DCMAKE_INSTALL_LIBDIR:PATH=%{_libdir} -DCMAKE_PREFIX_PATH:PATH=%{_prefix} @RPM_ALL_CMAKE_OPTIONS@
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}

mkdir -p $RPM_BUILD_ROOT/lib/systemd/system
mkdir -p $RPM_BUILD_ROOT/etc/sysconfig
install -p -m 0644 build/rpm/lp-autoanswer.service $RPM_BUILD_ROOT/lib/systemd/system
install -p -m 0644 build/rpm/lp-autoanswer.conf $RPM_BUILD_ROOT/etc/sysconfig
mv $RPM_BUILD_ROOT/etc/sysconfig/lp-autoanswer.conf $RPM_BUILD_ROOT/etc/sysconfig/lp-autoanswer

%check
#%{ctest_name} -V %{?_smp_mflags}

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files 
%defattr(-,root,root)
%doc ChangeLog.md COPYING README.md
%if @ENABLE_DAEMON@ || @ENABLE_CONSOLE_UI@ || @ENABLE_TOOLS@
%{_bindir}/*
%endif
%{_libdir}/*.so.*
#%{_mandir}/*
%{_datadir}/linphone
%{_datadir}/sounds/linphone
/etc/sysconfig/lp-autoanswer
/lib/systemd/system/lp-autoanswer.service

%files devel
%defattr(-,root,root)
%{_includedir}/linphone
%if @ENABLE_CXX_WRAPPER@
%{_includedir}/linphone++
%endif
%if @ENABLE_STATIC@
%{_libdir}/*.a
%endif
%if @ENABLE_SHARED@
%{_libdir}/*.so
%endif
%if @ENABLE_DOC@
%{_docdir}/linphone*/html
%{_docdir}/linphone*/xml
%endif
%{_datadir}/Linphone/cmake/*.cmake
%{_datadir}/LinphoneCxx/cmake/*.cmake
%{_datadir}/belr/grammars/cpim_grammar


%changelog
* Thu Jul 13 2017 jehan.monnier <jehan.monnier@linphone.org>
- cmake port
* Mon Aug 19 2013 jehan.monnier <jehan.monnier@linphone.org>
- Initial RPM release.
