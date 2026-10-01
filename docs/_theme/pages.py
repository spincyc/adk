"""Parts of pages made from the repository's own records.

    config.extra.library_version   the version in library.properties, which
                                   every page's footer and every lesson's
                                   PDF show, so a class can match the site
                                   to the ADK it installed
    <!-- changelog -->             CHANGELOG.md, in the changelog page
"""

import os
import re

from mkdocs.exceptions import PluginError

ROOT = os.path.dirname (os.path.dirname (os.path.dirname (os.path.abspath (__file__))))


def on_config (config):
    with open (os.path.join (ROOT, "library.properties"), encoding="utf-8") as file:
        version = re.search (r"^version\s*=\s*(\S+)", file.read (), re.M)
    if not version:
        raise PluginError ("library.properties gives no version")
    config.extra["library_version"] = version[1]
    return config


def on_page_markdown (markdown, page, config, files):
    if "<!-- changelog -->" in markdown:
        markdown = markdown.replace ("<!-- changelog -->", changelog ())
    return markdown


# The changelog's entries, under the page's own title.
def changelog ():
    with open (os.path.join (ROOT, "CHANGELOG.md"), encoding="utf-8") as file:
        text = file.read ()
    return re.sub (r"\A# .*\n", "", text).strip ()
