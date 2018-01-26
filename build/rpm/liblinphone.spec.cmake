# -*- rpm-spec -*-

## rpmbuild options
# These 2 lines are here because we can build the RPM for flexisip, in which
# case we prefix the entire installation so that we don't break compatibility
# with the user's libs.
# To compile with bc prefix, use rpmbuild -ba --with bc [SPEC]
%define                 pkg_name        %{?_with_bc:bc-liblinphone}%{!?_with_bc:liblinphone}
%{?_with_bc: %define    _prefix         /opt/belledonne-communications}

%define     pkg_prefix %{?_with_bc:bc-}%{!?_with_bc:}

# re-define some directories for older RPMBuild versions which don't. This messes up the doc/ dir
# taken from https://fedoraproject.org/wiki/Packaging:RPMMacros?rd=Packaging/RPMMacros
%define _datarootdir       %{_prefix}/share
%define _datadir           %{_datarootdir}
%define _docdir            %{_datadir}/doc

%define build_number @PROJECT_VERSION_BUILD@
%if %{build_number}
%define build_number_ext -%{build_number}

Name:           %{pkg_name}
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
%if %{?_with_soci:1}%{!?_with_soci:0}
Requires:	%{pkg_prefix}soci
%endif

%description
liblinphone is the voip sdk used by Linphone

%define		lime	%{?_without_lime:0}%{!?_without_lime:1}
%define		video	%{?_without_video:0}%{!?_without_video:1}


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

%prep
%setup -n %{name}-%{version}%{?build_number_ext}

%debug_package

%build
%{expand:%%%cmake_name} . -DCMAKE_INSTALL_LIBDIR:PATH=%{_libdir} -DCMAKE_PREFIX_PATH:PATH=%{_prefix} -DENABLE_VIDEO=%{video} -DENABLE_LIME=%{lime} -DENABLE_TOOLS=NO -DENABLE_CONSOLE_UI=NO -DENABLE_DAEMON=NO
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}

%check
#%{ctest_name} -V %{?_smp_mflags}

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files 
%defattr(-,root,root)
%doc AUTHORS ChangeLog COPYING NEWS README.md TODO
%{_libdir}/*.so.*
#%{_mandir}/*
%{_datadir}/linphone
%{_datadir}/sounds/linphone

%files devel
%defattr(-,root,root)
%{_includedir}/linphone
%{_includedir}/linphone++
%{_libdir}/*.a
%{_libdir}/*.so
%{_docdir}/linphone*/html
%{_docdir}/linphone*/xml
%{_datadir}/Linphone/cmake/*.cmake
%{_datadir}/LinphoneCxx/cmake/*.cmake
%{_datadir}/belr/grammars/cpim_grammar


%changelog
* Thu Jul 13 2017 jehan.monnier <jehan.monnier@linphone.org>
- cmake port
* Mon Aug 19 2013 jehan.monnier <jehan.monnier@linphone.org>
- Initial RPM release.
