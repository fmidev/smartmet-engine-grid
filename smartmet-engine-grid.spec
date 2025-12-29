%define DIRNAME grid
%define LIBNAME smartmet-%{DIRNAME}
%define SPECNAME smartmet-engine-%{DIRNAME}
Summary: SmartMet grid engine
Name: %{SPECNAME}
Version: 25.12.29
Release: 1%{?dist}.fmi
License: MIT
Group: SmartMet/Engines
URL: https://github.com/fmidev/smartmet-engine-grid
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

# https://fedoraproject.org/wiki/Changes/Broken_RPATH_will_fail_rpmbuild
%global __brp_check_rpaths %{nil}

%if 0%{?rhel} && 0%{rhel} < 9
%define smartmet_boost boost169
%else
%define smartmet_boost boost
%endif

BuildRequires: rpm-build
BuildRequires: gcc-c++
BuildRequires: smartmet-library-spine-devel >= 25.12.12
BuildRequires: smartmet-library-grid-content-devel >= 25.12.29
BuildRequires: smartmet-library-grid-files-devel >= 25.12.29
BuildRequires: smartmet-library-macgyver-devel >= 25.12.2
BuildRequires: make
BuildRequires: omniORB-devel >= 4.3.0
BuildRequires: %{smartmet_boost}-devel
BuildRequires: gdal310-devel
BuildRequires: bzip2-devel
BuildRequires: zlib-devel
BuildRequires: postgresql15-devel
Requires: %{smartmet_boost}-thread
Requires: smartmet-library-grid-content >= 25.12.29
Requires: smartmet-library-grid-files >= 25.12.29
Requires: smartmet-library-spine >= 25.12.12
Requires: smartmet-library-macgyver >= 25.12.2
Requires: omniORB-devel

%if 0%{?rhel} && 0%{rhel} == 8
Requires: libpqxx >= 1:7.7.0, libpqxx < 1:7.8.0
BuildRequires: libpqxx-devel >= 1:7.7.0, libpqxx-devel < 1:7.8.0
#TestRequires: libpqxx-devel >= 1:7.7.0, libpqxx-devel < 1:7.8.0
%else
%if 0%{?rhel} && 0%{rhel} == 9
Requires: libpqxx >= 1:7.9.0, libpqxx < 1:7.10.0
BuildRequires: libpqxx-devel >= 1:7.9.0, libpqxx-devel < 1:7.10.0
#TestRequires: libpqxx-devel >= 1:7.9.0, libpqxx-devel < 1:7.10.0
%else
%if 0%{?rhel} && 0%{rhel} >= 10
Requires: libpqxx >= 1:7.10.0, libpqxx < 1:7.11.0
BuildRequires: libpqxx-devel >= 1:7.10.0, libpqxx-devel < 1:7.11.0
#TestRequires: libpqxx-devel >= 1:7.10.0, libpqxx-devel < 1:7.11.0
%else
Requires: libpqxx
BuildRequires: libpqxx-devel
#TestRequires: libpqxx-devel
%endif
%endif
%endif

Provides: %{SPECNAME}

%description
SmartMet grid engine

%package -n %{SPECNAME}-devel
Summary: SmartMet %{SPECNAME} development headers
Group: SmartMet/Development
Provides: %{SPECNAME}-devel
Requires: smartmet-library-grid-content-devel >= 25.12.29
Requires: %{SPECNAME} = %{version}-%{release}
Requires: smartmet-library-spine-devel >= 25.12.12
Requires: smartmet-library-macgyver-devel >= 25.12.2
%description -n %{SPECNAME}-devel
SmartMet %{SPECNAME} development headers.

%package -n smartmet-engine-grid-test
Summary: SmartMet %{SPECNAME} - redis server with required data for testing purpose
Group: SmartMet/Development
Provides: smartmet-engine-grid-test
Requires: smartmet-library-grid-files >= 25.12.29
%description -n smartmet-engine-grid-test
SmartMet %{SPECNAME} - redis server with required data for testing purpose

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q -n %{SPECNAME}

%build -q -n %{SPECNAME}
make %{_smp_mflags}

%install
%makeinstall
mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/smartmet/engines

%clean
rm -rf $RPM_BUILD_ROOT

%files -n %{SPECNAME}
%defattr(0775,root,root,0775)
%{_datadir}/smartmet/engines/%{DIRNAME}.so

%files -n %{SPECNAME}-devel
%defattr(0664,root,root,0775)
%{_includedir}/smartmet/engines/%{DIRNAME}

%files -n smartmet-engine-grid-test
%defattr(0664,root,root,0775)
%{_datadir}/smartmet/test/grid
%attr(0755,root,root) %{_bindir}/smartmet-grid-test-config-creator

%changelog
* Mon Dec 29 2025 Mika Heiskanen <mika.heiskanen@fmi.fi> 25.12.29-1.fmi
- Repackaged due to library API changes

* Thu Nov 27 2025 Andris Pavēnis <andris.pavenis@fmi.fi> 25.11.27-1.fmi
- Repackage to smartmet-library-grid-files changes

* Wed Oct 15 2025 Mika Heiskanen <mika.heiskanen@fmi.fi> - 25.10.15-1.fmi
- Repackaged due to grid-files API changes

* Mon Aug 25 2025 Andris Pavēnis <andris.pavenis@fmi.fi> 25.8.25-1.fmi
- Repackage due to smartmet-engine-grid changes

* Tue Jun  3 2025 Mika Heiskanen <mika.heiskanen@fmi.fi> - 25.6.3-1.fmi
- Parameter alias mapping fix

* Fri May 30 2025 Mika Heiskanen <mika.heiskanen@fmi.fi> - 25.5.30-1.fmi
- Merging alias parameter details with requested parameter details
- Added process id to a temporary filename
- Added build-in parameter unit conversion support that can be used directly from plugins

* Tue Apr  8 2025 Mika Heiskanen <mika.heiskanen@fmi.fi> - 25.4.8-1.fmi
- Improved parameter auto-mappings and added build-in parameter unit conversions

* Wed Mar 19 2025 Mika Heiskanen <mika.heiskanen@fmi.fi> - 25.3.19-1.fmi
- Fixed parameter mappings
- Added possibility to request state information

* Thu Feb 20 2025 Andris Pavēnis <andris.pavenis@fmi.fi> 25.2.20-1.fmi
- Update to gdal-3.10, geos-3.13 and proj-9.5

* Fri Jan 24 2025 Mika Heiskanen <mika.heiskanen@fmi.fi> - 25.1.24-1.fmi
- Improved grid-admin related browsing

* Fri Jan 10 2025 Andris Pavēnis <andris.pavenis@fmi.fi> 25.1.10-1.fmi
- Admin/info request update

* Thu Jan  9 2025 Mika Heiskanen <mika.heiskanen@fmi.fi> - 25.1.9-1.fmi
- Added content info extension for parameter details

* Fri Nov  8 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.11.8-1.fmi
- Register admin requests to SmartMet::Spine::Reactor

* Wed Oct 16 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.10.16-1.fmi
- Added configuration information for local file caching

* Tue Sep  3 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.9.3-1.fmi
- New version due to smartmet-library-grid-files changes. Also code cleanup and simplification

* Wed Aug  7 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.8.7-2.fmi
- Fix requires (RHEL9, libpxx)

* Wed Aug  7 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.8.7-1.fmi
- Update to gdal-3.8, geos-3.12, proj-94 and fmt-11

* Wed Jul 17 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.7.17-1.fmi
- Do not link with libboost_filesystem

* Fri Jul 12 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.7.12-1.fmi
- Replace many boost library types with C++ standard library ones

* Mon Jun  3 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.6.3-2.fmi
- Repackaged due to ABI changes

* Mon Jun  3 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.6.3-1.fmi
- Repackaged due to ABI changes

* Thu May 16 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.5.16-1.fmi
- Clean up boost date-time uses

* Tue May  7 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.5.7-2.fmi
- Use Date library (https://github.com/HowardHinnant/date) instead of boost date_time

* Tue May  7 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.5.7-1.fmi
- Use Date library (https://github.com/HowardHinnant/date) instead of boost date_time

* Fri May  3 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.5.3-1.fmi
- Added height request features

* Thu Apr  4 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.4.4-1.fmi
- Added configuration parameter 'mappingLevelSimplification'

* Fri Feb 23 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> 24.2.23-1.fmi
- Full repackaging

* Tue Feb 20 2024 Mika Heiskanen <mheiskan@rhel8.dev.fmi.fi> - 24.2.20-1.fmi
- Added configuration settings for enabling caching grids to disk

* Mon Feb  5 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> 24.2.5-1.fmi
- Repackaged due to grid-files ABI changes

* Thu Jan  4 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.1.4-1.fmi
- Repackaged due to grid-files ABI changes

* Fri Dec 22 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.12.22-1.fmi
- Repackaged due to ThreadLock ABI changes

* Fri Nov 17 2023 Pertti Kinnia <pertti.kinnia@fmi.fi> - 23.11.17-1.fmi
- Repackaged due to API changes in grid-files and grid-content

* Fri Nov 10 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.11.10-1.fmi
- Repackaged due to API changes in grid-content

* Mon Oct 30 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.10.30-1.fmi
- Added support for sessions and authorization

* Thu Oct 12 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.10.12-1.fmi
- Repackage due to smartmet-library-grid-files and smartmet-library-grid-files changes

* Fri Sep 29 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.9.29-1.fmi
- Improved virtual grid features
- Initializing also unmapped parameters during the startup

* Mon Sep 11 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.9.11-1.fmi
- Repackaged due to ABI changes in grid-files

* Fri Jul 28 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.7.28-1.fmi
- Repackage due to bulk ABI changes in macgyver/newbase/spine

* Fri Jul 14 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.7.14-1.fmi
- Improved thread safety

* Tue Jul 11 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.7.11-1.fmi
- Use postgresql 15, gdal 3.5, geos 3.11 and proj-9.0

* Thu Jun  8 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.6.8-1.fmi
- Fixed mutex locking order issue

* Fri May 26 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.5.26-1.fmi
- Require omniORB >= 4.3.0

* Mon Apr 17 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.4.17-1.fmi
- Added PostgreSQL support

* Thu Mar  9 2023 Mika Heiskanen <mheiskan@rhel8.dev.fmi.fi> - 23.3.9-1.fmi
- Repackaged due to API changes in base libraries

* Mon Feb 20 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.2.20-1.fmi
- Repackaged due to base library changes

* Thu Jan 19 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.1.19-1.fmi
- Added enseble processing demo functions

* Mon Dec 12 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.12.12-1.fmi
- Added filesys support for the grid cache
- Added authentication support for the memory mapper

* Tue Nov  8 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.11.8-1.fmi
- Added more cache statistics

* Thu Oct 20 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.10.20-1.fmi
- Removed cache reporting related to the old content range cache

* Tue Oct 11 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.10.11-2.fmi
- Disable own memory mapping implementation in tests

* Tue Oct 11 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.10.11-1.fmi
- Re-enable grid-engine in tests

* Mon Oct 10 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.10.10-1.fmi
- Added server information
- Added memory mapping options

* Fri Sep  9 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.9.9-1.fmi
- Content cache sizes are now configurable

* Wed Aug 24 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.8.24-1.fmi
- Repackaged due to an ABI change in ServiceImplementation

* Tue Aug 23 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.8.23-1.fmi
- Repackaged due to ABI changes in grid libraries

* Fri Aug 19 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.8.19-1.fmi
- Added cache statistics

* Wed Aug 17 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.8.17-1.fmi
- Added support for EDR metadata queries

* Thu Aug  4 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.8.4-1.fmi
- Replaced Lua Math.pow calls with ^, Math.pow no longer exists in RHEL9

* Thu Jul 28 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.7.28-1.fmi
- Engine shutdown fixes

* Tue Jul 26 2022 Andris Pavenis <andris.pavenis@fmi.fi> 22.7.26-1.fmi
- Join thread before destroying engine object

* Fri Jun 17 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.6.17-1.fmi
- Add support for RHEL9. Update libpqxx to 7.7.0 (rhel8+) and fmt to 8.1.1

* Wed Jun  8 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.6.8-1.fmi
- Print true generation modification time

* Wed Jun  1 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.6.1-1.fmi
- Improved ETag calculation support

* Tue May 24 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.5.24-1.fmi
- Repackaged due to NFmiArea ABI changes

* Mon May 23 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.5.23-1.fmi
- Remove unnecessary linkage to newbase

* Thu May  5 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.5.5-1.fmi
- Commented out mapping files in the test package since /usr/share is not writable in CircleCI tests

* Mon Apr 25 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.4.25-1.fmi
- Improved shutdown logistics

* Wed Mar 30 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.3.30-1.fmi
- Repackaged due to grid-content ABI changes

* Mon Mar 28 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.3.28-1.fmi
- Repackaged due to grid-content ABI changes

* Tue Mar 22 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.3.22-1.fmi
- Thread safety fix

* Mon Mar 21 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.3.21-1.fmi
- Repackaged since grid-content ABI changed

* Tue Mar 15 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.3.15-1.fmi
- Fixed to ModificationLock require recompile

* Thu Mar 10 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.3.10-1.fmi
- Repackaged due to base library ABI changes

* Mon Mar  7 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.3.7-1.fmi
- Repackaged due to base library API changes

* Wed Mar  2 2022 Pertti Kinnia <pertti.kinnia@fmi.fi> - 22.3.2-1.fmi
- Duplicated parameter names removed

* Tue Mar  1 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.3.1-1.fmi
- Repackaged due to an ABI change in grid-content library

* Mon Feb 28 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.2.28-1.fmi
- Added geometry status information

* Wed Feb  9 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.2.9-1.fmi
- Minor updates

* Thu Jan 27 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.1.27-1.fmi
- Added modification time to admin query results

* Tue Jan 25 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.1.25-1.fmi
- Added unmapping of unused grid files to save kernel resources

* Fri Jan 21 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.1.21-1.fmi
- Repackage due to upgrade of packages from PGDG repo: gdal-3.4, geos-3.10, proj-8.2

* Thu Jan 13 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.1.13-1.fmi
- Minor fixes

* Wed Jan  5 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.1.5-1.fmi
- Added generation status updates

* Tue Dec  7 2021 Andris Pavēnis <andris.pavenis@fmi.fi> 21.12.7-1.fmi
- Update to postgresql 13 and gdal 3.3

* Mon Nov 15 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.11.15-1.fmi
- Added modification time information

* Thu Nov 11 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.11.11-1.fmi
- Removed point cache parameters

* Fri Oct 29 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.10.29-1.fmi
- Added querydata related admin features

* Tue Oct 19 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.10.19-1.fmi
- Repackaged due to grid base library API changes

* Mon Oct 11 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.10.11-1.fmi
- Simplified grid storage structures

* Mon Oct  4 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.10.4-1.fmi
- Simplified configuration files

* Wed Sep 15 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.9.15-1.fmi
- NetCDF support

* Tue Sep  7 2021 Andris Pavēnis <andris.pavenis@fmi.fi> 21.9.7-1.fmi
- Repackaged due to dependency changes (libconfig -> libconfig17)

* Tue Aug 31 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.8.31-1.fmi
- Repackaged due to ABI changes in Spine

* Wed Aug 18 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.8.18-1.fmi
- Improved origintime handling

* Tue Aug 17 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.8.17-1.fmi
- Use the new API for shutting down

* Mon Aug  2 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.8.2-1.fmi
- Repackaged since grid-content ABI changed by switching to boost::atomic_shared_ptr

* Thu Jul  8 2021 Andris Pavēnis <andris.pavenis@fmi.fi> 21.7.8-1.fmi
- Use libpqxx7 in RHEL8

* Tue Jun  8 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.6.8-1.fmi
- Repackaged due to memory saving ABI changes in base libraries
- Minor typo fix

* Tue Jun  1 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.6.1-1.fmi
- Updated to use the new library APIs

* Tue May 25 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.5.25-1.fmi
- Added a new method for fetching parameter alias information

* Tue Apr 27 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.4.27-1.fmi
- Fixed what=gridparameters query to handle an empty parameter table

* Fri Apr  2 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.4.2-1.fmi
- Renamed smartmet-test-grid to smartmet-engine-grid-test for CirleCI package prefix detection to work

* Thu Apr  1 2021 Pertti Kinnia <pertti.kinnia@fmi.fi> - 21.4.1-1.fmi
- Repackaged due to grid-files API changes

* Mon Mar 29 2021 Andris Pavēnis <andris.pavenis@fmi.fi> - 21.3.29-1.fmi
- Add test data and test config creator application

* Mon Mar 15 2021 Andris Pavēnis <andris.pavenis@fmi.fi> - 21.3.15-1.fmi
- Add missing dependencies for devel package

* Thu Mar 11 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.3.11-1.fmi
- Added methods for generating responses to admin plugin producer and parameter queries

* Wed Mar  3 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.3.3-1.fmi
- Added possibility to disable the grid-engine

* Tue Feb 16 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.2.16-1.fmi
- Repackaged due to base library ABI changes

* Wed Feb  3 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.2.3-1.fmi
- Repackaged due to time_t API changes in base libraries

* Wed Jan 27 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.27-1.fmi
- Repackaged due to base library ABI changes

* Tue Jan 19 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.19-2.fmi
- Performance improvements

* Tue Jan 19 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.19-1.fmi
- Fixed libpqxx dependencies

* Thu Jan 14 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.14-1.fmi
- Repackaged smartmet to resolve debuginfo issues

* Wed Jan 13 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.13-1.fmi
- Updated grid-files dependency

* Mon Jan 11 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.11-1.fmi
- Improved locking mechanism to avoid long lock durations

* Mon Jan  4 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.4-1.fmi
- Repackaged due to base library API changes

* Mon Dec 28 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.12.28-1.fmi
- New build system with updated GDAL and libpqxx dependencies

* Thu Dec  3 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.12.3-1.fmi
- Added parameters for Redis Content Server
- Added new LUA functions

* Mon Nov 30 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.11.30-1.fmi
- Refactored code, minor improvements

* Tue Nov 24 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.11.24-1.fmi
- Added engine browsing capability
- Minor fixes
- Config parameter 'producerAliasFiles' renamed to 'producerMappingFiles'
- Enabled configuration changes during the server execution
- Clearing the query cache when the producer search order changes
- Improved thread safety

* Thu Oct 22 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.10.22-1.fmi
- Performance improvements

* Tue Oct 20 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.10.20-1.fmi
- Rebuild due to libconfig upgrade to version 1.7.2

* Thu Oct 15 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.10.15-1.fmi
- Repackaged due to library ABI changes

* Wed Oct  7 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.10.7-1.fmi
- Repackaged due to library ABI changes

* Thu Oct  1 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.10.1-1.fmi
- Repackaged due to library ABI changes

* Wed Sep 23 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.23-1.fmi
- Use Fmi::Exception instead of Spine::Exception

* Mon Sep 21 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.21-1.fmi
- Disable query cache during its clean up

* Fri Sep 18 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.18-1.fmi
- Repackaged due to base library API changes

* Tue Sep 15 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.15-1.fmi
- Faster parameter mapping updates

* Mon Sep 14 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.14-1.fmi
- Repackaged due to base library ABI changes

* Tue Sep  8 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.8-1.fmi
- Another fix to query cache thread safety

* Mon Sep  7 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.7-1.fmi
- Fixed query cache thread safety

* Mon Aug 31 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.8.31-1.fmi
- Added query caching functionality

* Fri Aug 21 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.8.21-1.fmi
- Upgrade to fmt 6.2

* Tue Aug 18 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.8.18-1.fmi
- Repackaged due to grid library ABI changes

* Fri Aug 14 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.8.14-1.fmi
- Fixed initialization order of auto-files
- Added function to calculate fractiles from virtual grid files

* Mon Jun  8 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.6.8-1.fmi
- Updated Lua functions
- Added example virtual file definitions
- Upgraded libpqxx dependencies

* Fri May 15 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.5.15-1.fmi
- Added a configuration parameter for checking validity of memory mapped files

* Thu Apr 30 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.4.30-1.fmi
- Repackaged due to base library API changes

* Sat Apr 18 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.4.18-1.fmi
- Upgraded to Boost 1.69

* Fri Apr  3 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.4.3-1.fmi
- Parameter alias definitions separted into multiple files
- New configuration variables

* Wed Mar 11 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.3.11-1.fmi
- Memory locking is now configurable

* Thu Mar  5 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.3.5-1.fmi
- Added dem and land cover information access

* Tue Feb 25 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.2.25-1.fmi
- Disable request counter by default
- Disable debug log by default

* Wed Feb 19 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.2.19-1.fmi
- Added preloading and configuration files for it

* Wed Jan 29 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.1.29-1.fmi
- Added newbase alias names for producers
- Added a method for getting producers' official newbase names
- Repackaged due to library API changes

* Tue Jan 21 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.1.21-1.fmi
- Improved access to producer information

* Thu Jan 16 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.1.16-1.fmi
- Set maximum length of event list

* Wed Dec 11 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.12.11-1.fmi
- Repackaged due to small API changes in base libraries

* Wed Dec  4 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.12.4-1.fmi
- Repackaged due to changes in base libraries

* Fri Nov 22 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.11.22-1.fmi
- Repackaged due to API changes in grid-content library

* Wed Nov 20 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.11.20-1.fmi
- LUA function updates

* Thu Nov  7 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.11.7-1.fmi
- Added new producer aliases

* Fri Oct 25 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.10.25-1.fmi
- New engine methods

* Tue Oct  1 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.10.1-1.fmi
- Repackaged due to SmartMet library ABI changes

* Fri Sep 20 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.9.20-1.fmi
- Rebuild all with -fno-omit-frame-pointer

* Thu Sep 19 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.9.19-1.fmi
- New methods for parameter mapping
- Virtual grid file definitions

* Fri Aug  9 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.8.9-1.fmi
- Minor improvements

* Tue May 14 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.5.15-1.fmi
- Using original parameter values in new grib files

* Fri May 10 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.5.10-1.fmi
- Fixed missing member initialization

* Mon May  6 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.5.6-1.fmi
- Support for GRIB downloads

* Tue Mar 19 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.3.19-1.fmi
- Repackaged due to grid-files header changes

* Fri Mar 15 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.3.15-1.fmi
- Version update

* Fri Feb 15 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.2.15-1.fmi
- Version update

* Thu Jan 17 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.1.17-1.fmi
- Version update

* Wed Oct 24 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.10.24-1.fmi
- Downgraded spine dependency due to a bad rpm version number

* Mon Oct 15 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.10.15-1.fmi
- Added a LIST-function that is needed when information is queried by forecast number range
- Optional level-id field added into the producer alias definitions

* Wed Sep 26 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.9.26-1.fmi
- Version update

* Mon Sep 10 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.9.10-1.fmi
- Version update

* Thu Aug 30 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.30-1.fmi
- Silenced CodeChecker warnings

* Mon Aug 27 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.27-1.fmi
- Packaged latest version

* Thu Jun 14 2018 Roope Tervo <roope.tervo@fmi.fi> - 18.6.14-1.fmi
- Build for testing

* Thu Feb  8 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.2.8-1.fmi
- Initial build
