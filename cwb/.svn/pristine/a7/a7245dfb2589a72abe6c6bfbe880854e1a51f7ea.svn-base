Name:           cwb
Version:        3.5.0
Release:        1%{?dist}
Summary:        Indexing and query tools for very large text corpora
Group:          Applications/Databases

License:        GPLv2+
URL:            https://cwb.sourceforge.io/
Source0:        cwb-3.5.0-src.tar.gz
# TODO add URL for final release

ExclusiveArch:  x86_64
BuildRequires:  autoconf bison flex gcc pkgconfig glibc-devel glibc-headers make ncurses-devel pcre-devel glib2-devel readline-devel
Requires:       glibc glibc-common ncurses pcre readline

%description
The IMS Open Corpus Workbench (CWB) is a highly specialised read-only
database for large text corpora with linguistic annotations.  It uses
a compressed and platform-independent proprietary format to store
corpus data with token-level annotations and shallow structural markup.
Its central component is the corpus query processor CQP, a terminal
application designed for interactive work.  In addition to CQP, the
distribution includes a number of command-line utilities for encoding,
decoding and extracting frequency information.  Low-level access to
the corpus data is possible through the included C libary (CL).  Perl
bindings for the library and the query processor are available separately.

The IMS Open Corpus Workbench is distributed under the GNU Public License,
version 2.  Source and binary downloads as well as additional documentation
can be found on the CWB homepage at http://cwb.sourceforge.net/

%global debug_package %{nil}

%package devel
Summary: Headers and static library files for CWB and the Corpus Library (libcl)
Group: Development/Libraries

%description devel
Development files for the IMS Open Corpus Worbkench (CWB).
This includes header files for the Corpus Library (libcl) and CQi and the
static library for the libcl.

%prep
%setup -q -n %{name}-%{version}-src

%build
make clean all PLATFORM=linux-64 SITE=standard PREFIX=%{buildroot}/usr FINALPREFIX=/usr DEFAULT_REGISTRY=/usr/share/cwb/registry

%install
make install PLATFORM=linux-64 SITE=standard PREFIX=%{buildroot}/usr FINALPREFIX=/usr DEFAULT_REGISTRY=/usr/share/cwb/registry
mkdir -p %{buildroot}/usr/share/cwb/registry

%files
/usr/bin/*
/usr/lib/libcl.so
%dir /usr/share/cwb/registry
/usr/share/man/*

%files devel
/usr/include/*
/usr/lib/libcl.a

%changelog
* Mon Jul 18 2022 Andrew Hardie <andrewhardie@sourceforge.net> - 3.5.0-1
- Bumped to v3.5.0 
* Wed Jun 15 2022 Timm Weber <timm.weber@fau.de> - 3.4.37-1
- Added devel subpackage
* Thu May 19 2022 Timm Weber <timm.weber@fau.de> - 3.4.33-1
- First beta package
