CMD= pio
RUN_TARGET= $(CMD) run --target

run:
	$(CMD) run
buildfs:
	$(RUN_TARGET) buildfs
	$(RUN_TARGET) uploadfs
upload:
	$(CMD) run
	$(RUN_TARGET) upload
monitor:
	$(CMD)