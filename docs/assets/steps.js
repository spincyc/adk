// The printed steps remain the source of truth. Build along gives their
// current step room, while the complete drawing keeps the learner oriented.
(function () {
    "use strict";

    const ns = "http://www.w3.org/2000/svg";
    let serial = 0;

    function element (tag, name, text) {
        const node = document.createElement (tag);
        node.className = name;
        if (text !== undefined) node.textContent = text;
        return node;
    }

    function button (name, text, action) {
        const node = element ("button", name, text);
        node.type = "button";
        node.addEventListener ("click", action);
        return node;
    }

    function recall (key, revision, rows) {
        try {
            const saved = JSON.parse (localStorage.getItem (key));
            if (saved?.revision === revision && Array.isArray (saved.done)) {
                return new Set (saved.done.filter (id =>
                    rows.some (row => row.dataset.step === id)));
            }
        } catch (error) {
            // Blocked storage and old or damaged records start a fresh build.
        }
        return new Set ();
    }

    // Every copy owns its definitions, title and clip. Clipping to the
    // viewBox also clips the letterbox left by preserveAspectRatio.
    function drawingCopy (source) {
        const svg = source.cloneNode (true);
        const prefix = "adk-view-" + ++serial + "-";
        const ids = new Map ();
        for (const node of [svg, ...svg.querySelectorAll ("[id]")]) {
            if (node.id) {
                ids.set (node.id, prefix + node.id);
                node.id = prefix + node.id;
            }
        }
        for (const node of [svg, ...svg.querySelectorAll ("*")]) {
            for (const attr of [...node.attributes]) {
                let value = attr.value.replace (/url\(["']?#([^\s)'"]+)["']?\)/g,
                    (match, id) => ids.has (id) ? "url(#" + ids.get (id) + ")" : match);
                if (attr.localName === "href" && value.startsWith ("#")) {
                    value = "#" + (ids.get (value.slice (1)) || value.slice (1));
                }
                if (["aria-labelledby", "aria-describedby"].includes (attr.name)) {
                    value = value.split (/\s+/).map (id => ids.get (id) || id).join (" ");
                }
                if (value !== attr.value) node.setAttributeNS (attr.namespaceURI, attr.name, value);
            }
        }
        svg.classList.remove ("lighting");
        svg.removeAttribute ("width");
        svg.removeAttribute ("height");
        const layer = document.createElementNS (ns, "g");
        layer.append (...svg.childNodes);
        const defs = document.createElementNS (ns, "defs");
        const clip = document.createElementNS (ns, "clipPath");
        clip.id = prefix + "viewport";
        const rect = document.createElementNS (ns, "rect");
        clip.append (rect);
        defs.append (clip);
        layer.setAttribute ("clip-path", "url(#" + clip.id + ")");
        svg.append (defs, layer);
        function crop (box) {
            svg.setAttribute ("viewBox", box.join (" "));
            ["x", "y", "width", "height"].forEach ((name, i) => rect.setAttribute (name, box[i]));
        }
        crop (source.getAttribute ("viewBox").trim ().split (/\s+/).map (Number));
        return {svg, layer, crop};
    }

    function bounds (source, item) {
        const points = [];
        for (const group of source.querySelectorAll ("[data-item]")) {
            if (group.dataset.item !== item) continue;
            const box = group.getBBox ();
            if (!box.width && !box.height) continue;
            const matrix = source.getCTM ().inverse ().multiply (group.getCTM ());
            for (const x of [box.x, box.x + box.width]) {
                for (const y of [box.y, box.y + box.height]) {
                    points.push (new DOMPoint (x, y).matrixTransform (matrix));
                }
            }
        }
        if (!points.length) return null;
        const x = Math.min (...points.map (p => p.x));
        const y = Math.min (...points.map (p => p.y));
        return [x, y, Math.max (...points.map (p => p.x)) - x,
            Math.max (...points.map (p => p.y)) - y];
    }

    function workbench (steps, rows, source, done, changed, reset) {
        if (!source || !window.HTMLDialogElement) return null;
        const title = steps.dataset.title ||
            document.querySelector ("h1")?.textContent || "Build it";
        const dialog = element ("dialog", "build-workbench");
        const titleId = "adk-build-title-" + ++serial;
        dialog.setAttribute ("aria-labelledby", titleId);
        const shell = element ("div", "workbench-shell");
        const header = element ("header", "workbench-header");
        const heading = element ("div", "workbench-heading");
        heading.append (element ("p", "workbench-eyebrow", "ADK / Build along"));
        const name = element ("h2", "workbench-title", title);
        name.id = titleId;
        heading.append (name);
        const close = button ("workbench-close", "Back to lesson", () => dialog.close ());
        close.autofocus = true;
        header.append (heading, close);
        const stagebar = element ("nav", "workbench-stages");
        stagebar.setAttribute ("aria-label", "Build stages");
        const workspace = element ("div", "workbench-workspace");
        const instruction = element ("section", "workbench-instruction");
        const status = element ("p", "workbench-step-count");
        status.setAttribute ("role", "status");
        status.setAttribute ("aria-atomic", "true");
        const stepTitle = element ("h3", "workbench-step-title");
        stepTitle.tabIndex = -1;
        const action = element ("p", "workbench-action");
        const places = element ("div", "workbench-places");
        const note = element ("p", "workbench-note");
        instruction.append (status, stepTitle, action, places, note);
        const focus = element ("section", "workbench-panel workbench-focus");
        const focusTitle = element ("h3", "workbench-panel-title", "This step, up close");
        const views = element ("div", "workbench-views");
        const focusCaption = element ("p", "workbench-caption");
        focus.append (focusTitle, views, focusCaption);
        const overview = element ("section", "workbench-panel workbench-overview");
        const mapHeader = element ("div", "workbench-map-header");
        mapHeader.append (element ("h3", "workbench-panel-title", "The whole board"));
        const enlarge = button ("", "Enlarge map", () => {
            const expanded = overview.classList.toggle ("is-expanded");
            enlarge.setAttribute ("aria-pressed", String (expanded));
            enlarge.textContent = expanded ? "Small map" : "Enlarge map";
        });
        enlarge.setAttribute ("aria-pressed", "false");
        mapHeader.append (enlarge);
        overview.append (mapHeader);
        const map = drawingCopy (source);
        const mapWrap = element ("div", "workbench-map");
        mapWrap.append (map.svg);
        overview.append (mapWrap, element ("p", "workbench-caption",
            "Finished build · Mega on the left · current step marked"));
        const controls = element ("div", "workbench-controls");
        const previous = button ("", "Previous", () => select (current - 1));
        const next = button ("", "Next", () => select (current + 1));
        const tick = button ("workbench-done", "Done & next", () => {
            const id = rows[current].dataset.step;
            if (done.has (id)) {
                done.delete (id);
            } else {
                done.add (id);
                const first = rows.findIndex (row => !done.has (row.dataset.step));
                current = current + 1 < rows.length ? current + 1 : Math.max (0, first);
            }
            changed ();
            dialog.scrollTop = 0;
        });
        controls.append (previous, tick, next);
        const completion = element ("section", "workbench-completion");
        completion.tabIndex = -1;
        completion.append (element ("h3", "", "Every step checked"), element ("p", "",
            "Compare your wiring with the whole board, then return to the lesson for what to try."),
        button ("", "Review the steps", () => { reviewing = true; render (); }),
        button ("workbench-done", "Back to lesson", () => dialog.close ()));
        const side = element ("div", "workbench-side");
        side.append (instruction, controls);
        workspace.append (side, focus, overview, completion);
        const footer = element ("footer", "workbench-footer");
        footer.append (element ("p", "", "Keep USB unplugged while you build."));
        const all = element ("details", "workbench-all");
        all.append (element ("summary", "", "All steps"));
        const list = element ("ol", "workbench-list");
        const rowButtons = rows.map ((row, i) => {
            const li = element ("li", "");
            const jump = button ("", "", () => select (i));
            li.append (jump);
            list.append (li);
            return jump;
        });
        all.append (list);
        const clear = button ("workbench-reset", "Reset this build", () => {
            reset ();
            select (0);
        });
        footer.append (clear);
        const context = element ("details", "workbench-context");
        context.append (element ("summary", "", "Before you build · keep USB unplugged"));
        const contextBody = element ("div", "");
        const carry = steps.previousElementSibling;
        if (carry?.classList.contains ("build-context")) {
            contextBody.append (carry.cloneNode (true));
        }
        const buildSection = steps.closest (".build");
        const warnings = [];
        for (let node = buildSection?.previousElementSibling; node;
            node = node.previousElementSibling) {
            if (/^H[12]$/.test (node.tagName)) break;
            if (node.matches (".admonition.warning, .admonition.danger")) warnings.unshift (node);
        }
        for (const warning of warnings) contextBody.append (warning.cloneNode (true));
        context.open = Boolean (contextBody.children.length);
        contextBody.querySelectorAll ("[id]").forEach (node => node.removeAttribute ("id"));
        contextBody.addEventListener ("click", event => {
            if (event.target.closest ("a")) dialog.close ();
        });
        context.hidden = !contextBody.children.length;
        contextBody.append (button ("workbench-start", "Start building", () => {
            context.open = false;
            dialog.scrollTop = 0;
            stepTitle.focus ({preventScroll: true});
        }));
        context.append (contextBody);
        shell.append (header, context, stagebar, workspace, all, footer);
        dialog.append (shell);
        document.body.append (dialog);
        let current = Math.max (0, rows.findIndex (row => !done.has (row.dataset.step)));
        let reviewing = false;
        let opener;
        const stages = [...steps.querySelectorAll ("details.stage")];
        const stageButtons = stages.map (stage => {
            const jump = button ("", "", () => {
                const candidates = rows.filter (row => row.closest ("details.stage") === stage);
                select (rows.indexOf (candidates.find (row => !done.has (row.dataset.step)) ||
                    candidates[0]));
            });
            stagebar.append (jump);
            return jump;
        });

        function partName (row) {
            const what = row.querySelector (".what").cloneNode (true);
            what.querySelectorAll ("small").forEach (node => node.remove ());
            const color = row.querySelector (".wire") ?
                row.querySelector (".link small")?.textContent : null;
            const name = what.textContent.trim ();
            return color ?
                color[0].toUpperCase () + color.slice (1) + " " + name.toLowerCase () : name;
        }

        function highlight (copy, item, points) {
            copy.svg.classList.toggle ("workbench-lighting", Boolean (item));
            for (const group of copy.svg.querySelectorAll ("[data-item]")) {
                group.classList.remove ("lit", "next");
                group.classList.toggle ("workbench-active", group.dataset.item === item);
                group.classList.toggle ("workbench-placed", rows.some (row =>
                    row.dataset.item === group.dataset.item && done.has (row.dataset.step)));
            }
            for (const [x, y] of points) {
                const ring = document.createElementNS (ns, "circle");
                ring.setAttribute ("cx", x);
                ring.setAttribute ("cy", y);
                ring.setAttribute ("r", "6");
                ring.setAttribute ("class", "workbench-endpoint");
                copy.layer.append (ring);
            }
        }

        function select (index) {
            current = Math.max (0, Math.min (rows.length - 1, index));
            reviewing = true;
            const fromList = all.contains (document.activeElement);
            all.open = false;
            render ();
            if (fromList) stepTitle.focus ({preventScroll: true});
            dialog.scrollTop = 0;
        }

        function render () {
            if (!dialog.open) return;
            const row = rows[current];
            const complete = done.size === rows.length && !reviewing;
            completion.hidden = !complete;
            side.hidden = complete;
            for (const node of [instruction, focus, controls]) node.hidden = complete;
            workspace.classList.toggle ("is-complete", complete);
            if (complete) completion.focus ({preventScroll: true});
            clear.disabled = done.size === 0;
            status.textContent = "Step " + (current + 1) + " of " + rows.length + " · " +
                done.size + " checked";
            const stage = row.closest ("details.stage");
            stepTitle.textContent = partName (row);
            action.textContent = row.dataset.action ||
                stage.querySelector (".stage-title").textContent;
            const orientation = row.querySelector (".what small")?.textContent;
            note.textContent = [orientation && orientation.replace (/\.?$/, "."), row.dataset.care]
                .filter (Boolean).join (" ");
            places.replaceChildren (...[...row.querySelectorAll (".place")].map (place =>
                place.cloneNode (true)));
            places.classList.toggle ("many-places", places.children.length > 6);
            previous.disabled = current === 0;
            next.disabled = current === rows.length - 1;
            tick.textContent = done.has (row.dataset.step) ? "Mark unfinished" : "Done & next";
            if (!done.has (row.dataset.step) && done.size === rows.length - 1) {
                tick.textContent = "Done · finish build";
            }
            stages.forEach ((entry, i) => {
                const members = rows.filter (r => r.closest ("details.stage") === entry);
                const count = members.filter (r => done.has (r.dataset.step)).length;
                stageButtons[i].textContent = entry.querySelector (".stage-title").textContent +
                    " · " + count + "/" + members.length;
                stageButtons[i].setAttribute ("aria-current", entry === stage ? "step" : "false");
            });
            rowButtons.forEach ((jump, i) => {
                jump.textContent = (done.has (rows[i].dataset.step) ? "✓ " : "") +
                    (i + 1) + ". " + (rows[i].dataset.action || partName (rows[i]));
                jump.setAttribute ("aria-current", i === current ? "step" : "false");
            });
            const item = complete ? undefined : row.dataset.item;
            let points = [];
            try {
                points = JSON.parse (row.dataset.points || "[]");
            } catch (error) { /* No crop. */ }
            points = points.filter (p => Array.isArray (p) && p.length === 2 &&
                p.every (Number.isFinite));
            if (!item || complete) points = [];
            map.layer.querySelectorAll (".workbench-endpoint, .workbench-location")
                .forEach (node => node.remove ());
            highlight (map, item, points);
            views.replaceChildren ();
            const wire = Boolean (row.querySelector (".wire"));
            // A wire can take a long detour around other parts. Its close-up
            // belongs at the connections in the instruction, not that detour.
            const ends = wire && points.length === 2;
            const box = ends ? [Math.min (points[0][0], points[1][0]),
                Math.min (points[0][1], points[1][1]),
                Math.abs (points[0][0] - points[1][0]),
                Math.abs (points[0][1] - points[1][1])] : item ? bounds (source, item) : null;
            if (!box) {
                focusTitle.textContent = "Before the next part";
                views.append (element ("p", "workbench-no-part",
                    "This step has no part in the finished drawing. Follow the instruction, " +
                    "then use the whole board to check what stays."));
                focusCaption.textContent = "The map shows the finished build.";
                return;
            }
            focusTitle.textContent = "This step, up close";
            const split = ends && (box[2] > 230 || box[3] > 170);
            const width = Math.max (100, box[2] + 70);
            const height = Math.max (100, box[3] + 70);
            const crops = split ? points.map (([x, y]) => [x - 70, y - 65, 140, 130]) :
                [[box[0] + (box[2] - width) / 2, box[1] + (box[3] - height) / 2, width, height]];
            views.classList.toggle ("has-two-ends", split);
            crops.forEach ((crop, i) => {
                const figure = element ("figure", "workbench-crop");
                const copy = drawingCopy (source);
                copy.crop (crop);
                highlight (copy, item, points);
                copy.svg.setAttribute ("aria-label",
                    split ? "Wire end " + (i + 1) : partName (row));
                figure.append (copy.svg);
                if (split) {
                    const place = row.querySelectorAll (".place")[i];
                    const caption = element ("figcaption", "", "End " + (i + 1) + " · ");
                    if (place) caption.append (place.cloneNode (true));
                    figure.append (caption);
                }
                views.append (figure);
                const rect = document.createElementNS (ns, "rect");
                ["x", "y", "width", "height"].forEach ((name, j) =>
                    rect.setAttribute (name, crop[j]));
                rect.setAttribute ("class", "workbench-location");
                map.layer.append (rect);
            });
            focusCaption.textContent = split ?
                "Both ends, enlarged separately. Follow the whole wire in the map below." :
                ends ? "Connection points, enlarged. Follow the whole wire in the map below." :
                points.length ?
                "Same orientation as your board. The rings mark this step’s connection points." :
                "Same orientation as the whole board. The highlighted module is this step’s part.";
        }

        dialog.addEventListener ("close", () => {
            document.body.classList.remove ("building-along");
            opener?.focus ();
        });
        const launch = button ("build-launch", "Build along" +
            (steps.dataset.board ? " · " + title : ""), event => {
            opener = event.currentTarget;
            reviewing = false;
            current = Math.max (0, rows.findIndex (row => !done.has (row.dataset.step)));
            dialog.showModal ();
            document.body.classList.add ("building-along");
            render ();
        });
        launch.setAttribute ("aria-haspopup", "dialog");
        const intro = element ("div", "build-launcher");
        intro.append (launch, element ("span", "", "One step at a time, with a closer look."));
        steps.before (intro);
        return () => { reviewing = false; render (); };
    }

    function build (steps) {
        const board = steps.dataset.board;
        const rows = [...steps.querySelectorAll ("tr[data-step]")];
        if (!rows.length) return;
        const revision = steps.dataset.revision || rows.map (row => row.dataset.step).join (",");
        // The version separates old ordinal-only ticks from content-stable steps.
        const key = "adk-build-v2:" + location.pathname + ":" + board;
        const done = recall (key, revision, rows);
        const figures = [...document.querySelectorAll ("figure.bench-figure[data-board]")]
            .filter (figure => figure.dataset.board === board);
        const drawings = figures.map (figure => figure.querySelector ("svg.pencil-drawing"));
        const source = figures.find (figure => figure.classList.contains ("bench-bench"))
            ?.querySelector ("svg.pencil-drawing");
        let chosen = null;
        let passing = null;
        let updateWorkbench;
        const clear = button ("clear-ticks", "Clear the ticks", reset);
        steps.append (clear);

        function reset () {
            done.clear ();
            chosen = null;
            changed ();
        }

        function changed () {
            try {
                if (done.size) {
                    localStorage.setItem (key, JSON.stringify ({revision, done: [...done]}));
                } else {
                    localStorage.removeItem (key);
                }
            } catch (error) { /* A build also works without storage. */ }
            for (const row of rows) {
                const ticked = done.has (row.dataset.step);
                row.classList.toggle ("done", ticked);
                row.querySelector (".tick").setAttribute ("aria-pressed", String (ticked));
            }
            for (const stage of steps.querySelectorAll ("details.stage")) {
                const all = [...stage.querySelectorAll ("tr[data-step]")]
                    .every (row => done.has (row.dataset.step));
                if (all !== stage.classList.contains ("done")) stage.open = !all;
                stage.classList.toggle ("done", all);
            }
            clear.hidden = done.size === 0;
            chosen = null;
            passing = null;
            const next = done.size ? rows.find (row => !done.has (row.dataset.step)) : null;
            if (next) next.closest ("details.stage").open = true;
            refresh ();
            updateWorkbench?. ();
        }

        function refresh () {
            const looking = passing || chosen;
            const next = done.size ? rows.find (row => !done.has (row.dataset.step)) : null;
            for (const [name, row] of [["next", next], ["lit", looking]]) {
                for (const other of rows) other.classList.toggle (name, other === row);
                for (const drawing of drawings) {
                    for (const part of drawing.querySelectorAll ("[data-item]")) {
                        part.classList.toggle (name, Boolean (row?.dataset.item) &&
                            part.dataset.item === row.dataset.item);
                    }
                    drawing.classList.toggle ("lighting", Boolean (looking?.dataset.item));
                }
            }
        }

        for (const row of rows) {
            row.querySelector (".tick").addEventListener ("click", () => {
                if (!done.delete (row.dataset.step)) done.add (row.dataset.step);
                changed ();
            });
            row.addEventListener ("click", event => {
                if (event.target.closest ("button, a")) return;
                chosen = chosen === row ? null : row;
                refresh ();
            });
            for (const [event, active] of [["pointerenter", true], ["pointerleave", false],
                ["focusin", true], ["focusout", false]]) {
                row.addEventListener (event, e => {
                    if (event.startsWith ("pointer") && e.pointerType !== "mouse") return;
                    passing = active ? row : null;
                    refresh ();
                });
            }
        }
        updateWorkbench = workbench (steps, rows, source, done, changed, reset);
        changed ();
    }

    for (const steps of document.querySelectorAll (".build-steps[data-board]")) build (steps);
}) ();
