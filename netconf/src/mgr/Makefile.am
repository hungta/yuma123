lib_LTLIBRARIES = libyumamgr.la

libyumamgr_la_SOURCES = \
$(top_srcdir)/netconf/src/mgr/mgr.c \
$(top_srcdir)/netconf/src/mgr/mgr_cap.c \
$(top_srcdir)/netconf/src/mgr/mgr_hello.c \
$(top_srcdir)/netconf/src/mgr/mgr_io.c \
$(top_srcdir)/netconf/src/mgr/mgr_load.c \
$(top_srcdir)/netconf/src/mgr/mgr_not.c \
$(top_srcdir)/netconf/src/mgr/mgr_rpc.c \
$(top_srcdir)/netconf/src/mgr/mgr_ses.c \
$(top_srcdir)/netconf/src/mgr/mgr_signal.c \
$(top_srcdir)/netconf/src/mgr/mgr_top.c \
$(top_srcdir)/netconf/src/mgr/mgr_val_parse.c \
$(top_srcdir)/netconf/src/mgr/mgr_xml.c


libyumamgr_la_CPPFLAGS = -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
libyumamgr_la_LDFLAGS = -version-info 2:0:0 $(top_builddir)/netconf/src/ncx/libyumancx.la -lxml2 -lrt -lssh2
