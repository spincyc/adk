// A downloaded PDF must lead back to the published course even when it
// was printed from a local preview. Keep the browser's links unchanged.
(function () {
    "use strict";

    const saved = new Map ();

    window.addEventListener ("beforeprint", () => {
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
        for (const [link, href] of saved) link.setAttribute ("href", href);
        saved.clear ();
    });
} ());
