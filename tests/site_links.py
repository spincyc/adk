#!/usr/bin/env python3
"""Check the built site's own links: every page and file a link names, every
anchor, and no id twice in a page.

MkDocs checks the links a page's Markdown makes, but not the ones the build
hooks and the theme write into the HTML: a lesson header's sections, a
drawing's <use> and url(#...) references, a PDF's link. So this reads every
built page and fails on a link within the site to a file that isn't there,
a #fragment that names no element of its page, or an id two elements
share, where a link or a drawing could find the wrong one. Links to other
sites are left to them. Lessons' PDFs are printed after the site is built,
by make pdf, so a link to one counts when its lesson's page exists.
"""

import argparse
import re
import sys
from concurrent.futures import ProcessPoolExecutor
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import unquote, urlsplit

sys.dont_write_bytecode = True

ROOT      = Path (__file__).resolve ().parent.parent
SITE_URL  = re.search (r"^site_url:\s*(\S+)",
                       (ROOT / "mkdocs.yml").read_text (encoding="utf-8"), re.M)[1]
SITE_PATH = urlsplit (SITE_URL).path        # "/adk/", where GitHub Pages serves it
LINKS     = {"a": "href", "area": "href", "link": "href", "img": "src", "script": "src",
             "source": "src", "iframe": "src", "use": "href", "image": "href"}
URL_REF   = re.compile (r"url\(\s*['\"]?#([^)'\"\s]+)")


class Page (HTMLParser):
    def __init__ (self):
        super ().__init__ (convert_charrefs=True)
        self.ids, self.twice, self.links = set (), set (), []

    def handle_starttag (self, tag, attrs):
        attrs = dict (attrs)
        name = attrs.get ("id")
        if name is not None:
            if name in self.ids:
                self.twice.add (name)
            self.ids.add (name)
        if tag == "a" and attrs.get ("name"):
            self.ids.add (attrs["name"])
        link = attrs.get (LINKS.get (tag, "")) or \
            (attrs.get ("xlink:href") if tag in ("use", "image") else None)
        if link and attrs.get ("rel") not in ("preconnect", "dns-prefetch"):
            self.links.append (link)
        # A drawing's filter="url(#bench-pencil)" and the like.
        self.links += ["#" + ref for value in attrs.values () if value and "url(" in value
                       for ref in URL_REF.findall (value)]

    handle_startendtag = handle_starttag


def read (path):
    page = Page ()
    page.feed (path.read_text (encoding="utf-8"))
    page.close ()
    return path, page.ids, page.twice, page.links


def target (site, page, link):
    """The file within the site that a link names, and its fragment; or None
    for a link to another site."""
    if link.startswith (SITE_URL):
        link = SITE_PATH + link.removeprefix (SITE_URL)
    parts = urlsplit (link)
    if parts.scheme or parts.netloc:
        return None
    if not parts.path:
        return page, unquote (parts.fragment)
    if parts.path.startswith ("/"):
        # From the server's root, which is GitHub's, not this site's.
        if not (parts.path + "/").startswith (SITE_PATH):
            return site.parent, ""
        path = site / unquote (parts.path.removeprefix (SITE_PATH))
    else:
        path = page.parent / unquote (parts.path)
    path = path.resolve ()
    if parts.path.endswith ("/") or path.is_dir ():
        path = path / "index.html"
    return path, unquote (parts.fragment)


def problems (site, pages):
    for path, (ids, twice, links) in pages.items ():
        where = path.relative_to (site)
        for name in sorted (twice):
            yield f'{where}: id "{name}" is used more than once'
        for link in dict.fromkeys (links):
            found = target (site, path, link)
            if found is None:
                continue
            file, fragment = found
            printed = file.parent == site / "pdf" and file.suffix == ".pdf" and \
                (site / "lessons" / file.stem / "index.html").is_file ()
            if not file.is_relative_to (site):
                yield f"{where}: {link} is outside the site"
            elif not file.exists () and not printed:
                yield f"{where}: {link} names no file"
            elif fragment and file in pages and fragment not in pages[file][0]:
                yield f"{where}: {link} names nothing on its page"


def main ():
    parser = argparse.ArgumentParser (description=__doc__.split ("\n\n")[0])
    parser.add_argument ("site_dir", type=Path)
    args = parser.parse_args ()
    site = args.site_dir.resolve ()
    paths = sorted (site.rglob ("*.html"))
    if not paths:
        parser.error (f"no pages in {site}")

    with ProcessPoolExecutor () as pool:
        pages = {path: rest for path, *rest in pool.map (read, paths, chunksize=4)}
    found = list (problems (site, pages))
    if found:
        sys.exit (f"{len (found)} broken link{'s' if len (found) > 1 else ''} in the site:\n  " +
                  "\n  ".join (found))
    print (f"Links, anchors and ids in {len (paths)} pages pass")


if __name__ == "__main__":
    main ()
