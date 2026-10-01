"""Parts of pages made from the repository's own records.

    config.extra.library_version   the version in library.properties, which
                                   every page's footer and every lesson's
                                   PDF show, so a class can match the site
                                   to the ADK it installed
    <!-- changelog -->             CHANGELOG.md, in the changelog page
    <!-- hours 1-36 -->            how long Lessons 1 to 36 take together,
                                   by their global numbers (E01 is 56)
    <!-- session plan projects --> each arc of a track as a session: its
                                   lessons, their times and the ideas each
                                   teaches, for the teacher guide

The times are the estimates in each lesson's front matter, such as
"45 minutes", "1 hour" or "1½ hours". A time this can't read stops the
build, so a total is never quietly short.
"""

import os
import re

import yaml
from mkdocs.exceptions import PluginError

ROOT    = os.path.dirname (os.path.dirname (os.path.dirname (os.path.abspath (__file__))))
COURSE  = os.path.join (ROOT, "docs", "_theme", "course.yml")
LESSONS = os.path.join (ROOT, "docs", "lessons")
UNITS   = {"minute": 1, "minutes": 1, "min": 1, "hour": 60, "hours": 60, "h": 60}
TIME    = re.compile (r"(\d+(?:\.\d+)?)?([½¼¾])?\s*(minutes?|min|hours?|h)\b")


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
    if "<!-- hours " in markdown or "<!-- session plan " in markdown:
        here = os.path.dirname (page.file.src_uri)
        lessons = course ()
        markdown = re.sub (r"<!-- hours (\d+)-(\d+) -->",
                           lambda match: duration (sum (lesson["minutes"] for lesson in lessons
                                                        if int (match[1]) <= lesson["number"]
                                                        <= int (match[2]))),
                           markdown)
        markdown = re.sub (r"<!-- session plan (\w+) -->",
                           lambda match: session_plan (lessons, match[1], here), markdown)
    return markdown


# The changelog's entries, under the page's own title.
def changelog ():
    with open (os.path.join (ROOT, "CHANGELOG.md"), encoding="utf-8") as file:
        text = file.read ()
    return re.sub (r"\A# .*\n", "", text).strip ()


# Every written lesson in course order, with its track, its visible label,
# its arc and what its front matter says.
def course ():
    with open (COURSE, encoding="utf-8") as file:
        arcs = yaml.safe_load (file)
    lessons, counts = [], {}
    for number, (arc, lesson) in enumerate (((arc, lesson) for arc in arcs
                                             for lesson in arc["lessons"]), 1):
        track = arc.get ("track", "projects")
        counts[track] = counts.get (track, 0) + 1
        path = os.path.join (LESSONS, lesson["slug"], "index.md")
        if not os.path.exists (path):
            continue
        meta = front_matter (path)
        lessons.append (dict (lesson, number=number, track=track, arc=arc["arc"],
                              label=(f"E{counts[track]:02d}" if track == "electricity"
                                     else str (counts[track])),
                              time=meta["time"], minutes=minutes (meta["time"], path),
                              ideas=meta.get ("ideas", [])))
    return lessons


def front_matter (path):
    with open (path, encoding="utf-8") as file:
        match = re.match (r"---\n(.*?)\n---\n", file.read (), re.S)
    if not match:
        raise PluginError (f"{os.path.relpath (path, ROOT)}: no front matter")
    return yaml.safe_load (match[1])


# "45 minutes", "1 hour", "1½ hours", "1 hour 30 minutes": in minutes.
def minutes (time, path):
    total, rest = 0, str (time)
    for match in TIME.finditer (rest):
        whole, part, unit = match.groups ()
        if not (whole or part):
            break
        amount = float (whole or 0) + {"½": 0.5, "¼": 0.25, "¾": 0.75}.get (part, 0)
        total += amount * UNITS[unit]
        rest = rest.replace (match[0], "", 1)
    if not total or rest.strip ():
        raise PluginError (f"{os.path.relpath (path, ROOT)}: can't read its time, {time!r}; "
                           f"write it as \"45 minutes\", \"1 hour\" or \"1½ hours\"")
    return round (total)


def duration (total):
    hours, mins = divmod (total, 60)
    return " ".join (([f"{hours} h"] if hours else []) +
                     ([f"{mins} min"] if mins or not hours else []))


# A heading for each arc with its total time, and a table of its lessons.
def session_plan (lessons, track, here):
    plan, arcs = [], {}
    for lesson in lessons:
        if lesson["track"] == track:
            arcs.setdefault (lesson["arc"], []).append (lesson)
    if not arcs:
        raise PluginError (f"<!-- session plan {track} --> names no track with written lessons")
    for arc, members in arcs.items ():
        first, last = members[0]["label"], members[-1]["label"]
        span = first if first == last else f"{first}–{last}"
        name = arc.removeprefix ("Electricity · ")
        word = ("Lesson " if first == last else "Lessons ") if track == "projects" else ""
        heading = f"{name} · {word}{span}"
        plan += [f"### {heading}", "",
                 f"{duration (sum (lesson['minutes'] for lesson in members))} in all.", "",
                 "| Lesson | Time | Objectives |", "|---|---|---|"]
        for lesson in members:
            link = os.path.relpath (f"lessons/{lesson['slug']}/index.md", here or ".")
            star = "★ " if lesson.get ("project") else ""
            ideas = "<br>".join (str (idea).replace ("|", "\\|") for idea in lesson["ideas"])
            plan.append (f"| {lesson['label']} · {star}[{lesson['title']}]({link}) | "
                         f"{lesson['time']} | {ideas} |")
        plan.append ("")
    return "\n".join (plan)
