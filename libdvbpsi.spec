%define name		libdvbpsi
%define version		0.1.2
%define release		2mdk

%define major		1
%define lib_name	%{name}%{major}

Summary:	A library for decoding and generating MPEG 2 and DVB PSI sections.
Name:		%{name}
Version:	%{version}
Release:	%{release}
License:	GPL
URL:		http://www.videolan.org/libdvbpsi/
Group:		System/Libraries
Source:		http://www.videolan.org/pub/videolan/libdvbpsi/%{version}/%{name}-%{version}.tar.gz
BuildRoot:	%_tmppath/%name-%version-%release-root

%description
libdvbpsi is a simple library designed for MPEG 2 TS and DVB PSI tables
decoding and generating. The important features are:
 * PAT decoder and genarator.
 * PMT decoder and generator.

%package -n %{lib_name}
Summary:	A library for decoding and generating MPEG 2 and DVB PSI sections.
Group:		System/Libraries
Provides:	%name

%description -n %{lib_name}
libdvbpsi is a simple library designed for MPEG 2 TS and DVB PSI tables
decoding and generating. The important features are:
 * PAT decoder and genarator.
 * PMT decoder and generator.

%package -n %{lib_name}-devel
Summary:	Development tools for programs which will use the libdvbpsi library.
Group:		Development/C
Provides:	%name-devel
Requires:	%{lib_name} = %{version}

%description -n %{lib_name}-devel
The %{name}-devel package includes the header files and static libraries
necessary for developing programs which will manipulate MPEG 2 and DVB PSI
information using the %{name} library.

If you are going to develop programs which will manipulate MPEG 2 and DVB PSI
information you should install %{name}-devel.  You'll also need to have
the %name package installed.


%prep
%setup -q

%build
%configure --enable-release
%make 

%install
%makeinstall

%clean
rm -rf %buildroot

%post -n %{lib_name} -p /sbin/ldconfig

%postun -n %{lib_name} -p /sbin/ldconfig

%files -n %{lib_name}
%defattr(-,root,root,-)
%doc AUTHORS README COPYING ChangeLog
%{_libdir}/*.so.*

%files -n %{lib_name}-devel
%defattr(-,root,root)
%doc COPYING
%{_libdir}/*a
%{_libdir}/*.so
%{_includedir}/*

%changelog
* Fri Dec 13 2002 Yves Duret <yves@zarb.org> 0.1.2-2mdk
- s#Copyright#License#
- include the libtool .la files.
- use macros.
- update URL: tag.

* Fri Oct 11 2002 Samuel Hocevar <sam@zoy.org>
- 0.1.2 release.

* Sat May 18 2002 Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
- 0.1.1 release.

* Mon Apr 8 2002 Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
- split into two separate packages.

* Thu Apr 4 2002 Jean-Paul Saman <saman@natlab.research.philips.com>
- first version of package for redhat systems.

