// A lesson's build steps (hooks.py writes them): tick a step off, and see
// the part it adds lit up in the board's drawings. The ticks are kept in
// this browser, a set per page and board; a stage whose steps are all
// ticked folds away. Where there is room, the drawing above the steps stays
// in sight while they scroll.

(function () {
    "use strict";

    function recall (key) {
        try {
            return new Set (JSON.parse (localStorage.getItem (key) || "[]"));
        } catch (error) {
            return new Set ();
        }
    }

    function remember (key, done) {
        try {
            if (done.size) {
                localStorage.setItem (key, JSON.stringify ([...done]));
            } else {
                localStorage.removeItem (key);
            }
        } catch (error) {
            // Private windows and blocked storage just forget.
        }
    }

    function build (steps) {
        const board = steps.dataset.board;
        const key = "adk-steps:" + location.pathname + ":" + board;
        const done = recall (key);
        const drawings = document.querySelectorAll (
            'figure.bench-figure[data-board="' + board + '"] svg.pencil-drawing');
        const rows = [...steps.querySelectorAll ("tr[data-step]")];
        let chosen = null;

        const clear = document.createElement ("button");
        clear.type = "button";
        clear.className = "clear-ticks";
        clear.textContent = "Clear the ticks";
        steps.append (clear);

        // A stage whose steps are all ticked folds away, and opens again
        // when one is unticked.
        function show (row) {
            const ticked = done.has (row.dataset.step);
            row.classList.toggle ("done", ticked);
            row.querySelector (".tick").setAttribute ("aria-pressed", String (ticked));
            const stage = row.closest ("details.stage");
            const all = [...stage.querySelectorAll ("tr[data-step]")]
                .every (other => done.has (other.dataset.step));
            if (all !== stage.classList.contains ("done")) {
                stage.open = !all;
            }
            stage.classList.toggle ("done", all);
            clear.hidden = done.size === 0;
        }

        function light (row) {
            const item = row && row.dataset.item;
            for (const other of rows) {
                other.classList.toggle ("lit", other === row && item !== undefined);
            }
            for (const drawing of drawings) {
                drawing.classList.toggle ("lighting", item !== undefined);
                for (const part of drawing.querySelectorAll ("[data-item]")) {
                    part.classList.toggle ("lit", part.dataset.item === item);
                }
            }
        }

        for (const row of rows) {
            show (row);
            row.querySelector (".tick").addEventListener ("click", () => {
                const step = row.dataset.step;
                if (!done.delete (step)) {
                    done.add (step);
                }
                remember (key, done);
                show (row);
            });
            // A tap chooses a step to light up, and a second tap lets it
            // go; a pointer passing over lights each step for a moment.
            row.addEventListener ("click", event => {
                if (event.target.closest ("button, a")) {
                    return;
                }
                chosen = chosen === row ? null : row;
                light (chosen);
            });
            row.addEventListener ("pointerenter", event => {
                if (event.pointerType === "mouse") {
                    light (row);
                }
            });
            row.addEventListener ("pointerleave", event => {
                if (event.pointerType === "mouse") {
                    light (chosen);
                }
            });
            row.addEventListener ("focusin", () => light (row));
            row.addEventListener ("focusout", () => light (chosen));
        }

        clear.addEventListener ("click", () => {
            done.clear ();
            remember (key, done);
            for (const row of rows) {
                show (row);
            }
        });
    }

    // The drawing stays above the steps only where it leaves them most of
    // the screen.
    function pin () {
        for (const figure of document.querySelectorAll (".build > .bench-figure")) {
            figure.classList.remove ("pinned");
            const room = window.innerWidth >= 960 &&
                figure.getBoundingClientRect ().height <= window.innerHeight * 0.45;
            figure.classList.toggle ("pinned", room);
        }
    }

    for (const steps of document.querySelectorAll (".build-steps[data-board]")) {
        build (steps);
    }
    pin ();
    window.addEventListener ("resize", pin);
}) ();
