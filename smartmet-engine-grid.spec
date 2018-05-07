%define DIRNAME grid
%define LIBNAME smartmet-%{DIRNAME}
%define SPECNAME smartmet-engine-%{DIRNAME}
Summary: SmartMet grid engine
Name: %{SPECNAME}
Version: 18.2.8
Release: 1%{?dist}.fmi
License: MIT
Group: SmartMet/Engines
URL: https://github.com/fmidev/smartmet-engine-grid
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: rpm-build
BuildRequires: gcc-c++
BuildRequires: libconfig-devel
BuildRequires: libpqxx-devel
BuildRequires: smartmet-library-grid-content-devel >= 18.2.8
BuildRequires: smartmet-library-grid-files-devel >= 18.1.25
BuildRequires: smartmet-library-spine-devel >= 18.1.15
BuildRequires: make
BuildRequires: omniORB-devel
Requires: boost-thread
Requires: libconfig
Requires: libpqxx-devel
Requires: smartmet-library-grid-content >= 18.2.8
Requires: smartmet-library-grid-files >= 18.1.25
Requires: smartmet-library-spine >= 18.1.15
Requires: omniORB-devel
Provides: %{SPECNAME}

%description
SmartMet grid engine

%package -n %{SPECNAME}-devel
Summary: SmartMet %{SPECNAME} development headers
Group: SmartMet/Development
Provides: %{SPECNAME}-devel
%description -n %{SPECNAME}-devel
SmartMet %{SPECNAME} development headers.

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

%changelog
* Thu Feb  8 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.2.8-1.fmi
- Initial build
