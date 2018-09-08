#
# spec file for package fdpp
#

Name: fdpp
Version: 0.1beta1
Release: 1%{?dist}
Summary: DOS compatibility layer

Group: System/Emulator

License: GPL-3.0+
URL: http://www.github.com/stsp/fdpp
Source0: %{name}-%{version}.tar.gz

BuildRequires: bison
BuildRequires: flex
BuildRequires: sed
BuildRequires: bash
BuildRequires: clang
BuildRequires: nasm
BuildRequires: libstdc++-devel

%description
fdpp is a 64-bit DOS.
It is based on a FreeDOS kernel ported to modern C++.

%prep
%setup -q

%build
make PREFIX=%{_prefix} LIBDIR=%{_libdir} %{?_smp_mflags}

%check

%install
make DESTDIR=%{buildroot} PREFIX=%{_prefix} LIBDIR=%{_libdir} install

%files
%defattr(-,root,root)
%{_libdir}/fdpp/libfdpp.so
%{_libdir}/pkgconfig/fdpp.pc
%{_includedir}/fdpp/thunks.h
%{_datadir}/fdpp/fdppkrnl.sys

%changelog
