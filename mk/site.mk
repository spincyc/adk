MKDOCS       ?= mkdocs
SITE_TOOL    := scripts/site
SITE_PORT    ?= 8000

.PHONY: site site-check site-serve

site:
	@$(SITE_TOOL) build

site-check:
	@$(SITE_TOOL) check

site-serve:
	@$(SITE_TOOL) serve --port "$(SITE_PORT)"

check: site-check
