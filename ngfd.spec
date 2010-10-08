Name:    ngfd
Version: 0.23
Release: 1
Summary: Non-graphic feedback service for sounds, vibration, led and backlight
Group:   System/Daemons
License: LGPL 2.1
#URL:
Source0: %{name}-%{version}.tar.gz

BuildRequires: pkgconfig(glib-2.0) >= 2.18.0
BuildRequires: pkgconfig(dbus-1) >= 1.0.2
BuildRequires: pkgconfig(dbus-glib-1)
BuildRequires: pkgconfig(libpulse)
BuildRequires: pkgconfig(gstreamer-0.10)
BuildRequires: pkgconfig(gstreamer-controller-0.10)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(gobject-2.0)
BuildRequires: pkgconfig(gthread-2.0)
BuildRequires: pkgconfig(profile)
BuildRequires: pkgconfig(sndfile)
BuildRequires: pkgconfig(check)
BuildRequires: pkgconfig(mce) >= 1.10.21

%description
This package contains the daemon servicing the non-graphical feedback
requests.
 
%prep
%setup -q

%build
%autogen
%configure --disable-vibrator
make %{?_smp_mflags}

%install
make DESTDIR=%{buildroot} install

%files
%defattr(-,root,root,-)
%doc COPYING
%config(noreplace) %{_sysconfdir}/dbus-1/system.d/%{name}.conf
%config(noreplace) %{_datadir}/dbus-1/services/com.nokia.NonGraphicFeedback1.Backend.service
%config(noreplace) %{_sysconfdir}/ngf/ngf.ini
/usr/bin/%{name}
