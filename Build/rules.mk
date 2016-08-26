#
# A simple way to generate depend file(.depend) for .c
#

_depend: $(obj).depend

$(obj).depend: $(BUILD_DIR)/config.mk $(SRC_C)
	@rm -f $@
	@for f in $(SRC_C); do \
		g=`basename $$f | sed -e 's/\(.*\)\.\w/\1.o/'`; \
		$(CC) -MM $(CFLAGS) -E -MQ $(_obj)$$g $$f >> $@ ; \
	done
