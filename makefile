.PHONY: clean dist

dist: clean
	tar -hzcf "$(CURDIR).tar.gz" hashtable/* main/* holdall/* option/* makefile

clean:
	$(MAKE) -C main clean
