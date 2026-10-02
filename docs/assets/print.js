// A downloaded PDF must lead back to the published course even when it
// was printed from a local preview. Keep the browser's links unchanged.
(function () {
    "use strict";

    const saved = new Map ();
    const shortCode = new Set ();

    window.addEventListener ("beforeprint", () => {
        // The print code size fits at least 40 lines on a sheet. Preserve
        // complete examples up to half that length, including blank lines,
        // without making a long project sketch an unbreakable block.
        for (const block of document.querySelectorAll (".highlight:has(> .filename)")) {
            const code = block.querySelector ("pre > code");
            if (code && code.textContent.trimEnd ().split ("\n").length <= 20) {
                block.classList.add ("print-short-code");
                shortCode.add (block);
            }
        }

        const canonical = document.querySelector ('link[rel="canonical"]')?.href;
        if (!canonical) return;

        for (const link of document.querySelectorAll ("a[href]")) {
            const href = link.getAttribute ("href");
            // Fragment links, including code line numbers, belong in the
            // current PDF. Absolute links already name their destination.
            if (!href || href.startsWith ("#") || /^[a-z][a-z0-9+.-]*:/i.test (href)) continue;
            if (!saved.has (link)) saved.set (link, href);
            link.setAttribute ("href", new URL (href, canonical).href);
        }
    });

    window.addEventListener ("afterprint", () => {
        for (const block of shortCode) block.classList.remove ("print-short-code");
        shortCode.clear ();
        for (const [link, href] of saved) link.setAttribute ("href", href);
        saved.clear ();
    });
} ());
