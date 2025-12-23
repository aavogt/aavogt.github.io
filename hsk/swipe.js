// Config
const PRELOAD_RANGE = 4; // ensure [current-PRELOAD_RANGE .. current+PRELOAD_RANGE]
const KEEP_RANGE = 7;    // delete tables farther than this distance from current
const MAXPAGE = 200; //  XXX

const hc = document.getElementById('hContainer');
const ind = document.querySelector('.scroll-indicator');

// Track which indices are present/loading
const loading = new Set();

function getIndicesInDomOrder() {
  return Array.from(hc.children).map(ch => parseInt(ch.dataset.index));
}

function getVisibleDomOffset() {
  const w = hc.clientWidth || 1;
  return Math.round(hc.scrollLeft / w);
}

function getVisibleIndex() {
  const indices = getIndicesInDomOrder();
  const off = getVisibleDomOffset();
  return indices[off] ?? indices[0] ?? 1;
}

function getSectionByIndex(i) {
  return Array.from(hc.children).find(ch => parseInt(ch.dataset.index) === i) || null;
}

// formerly async but I don't want asyncify and the jsapi option didn't seem to work
// probably because I didn't generate async functions like Module.ccall("func", "number", [], [], {async: true}).then(result => {
//   console.log("js_func: " + result);
// });
function fillSectionAsync(index, sec) {
  if (!sec.isConnected) return;

  const tbody = sec.querySelector('tbody');
  tbody.innerHTML = `<tr><td colspan=5>Loading table ${index}...</td></tr>`

  tbody.ariaBusy = true;
  tbody.innerHTML = '';
  fill_section(index);
  tbody.ariaBusy = false;
}

function setVisibleIndex(i, { behavior = 'smooth' } = {}) {
  ensureSection(i);
  const el = getSectionByIndex(i);
  if (!el) return;

  const indices = getIndicesInDomOrder();
  const pos = indices.indexOf(i);
  if (pos >= 0) {
    const w = hc.clientWidth || 1;
    hc.scrollTo({ left: pos * w, behavior });
  } else {
    el.scrollIntoView({ behavior, inline: 'start', block: 'nearest' });
  }

  ind.textContent = `Horizontal: Table ${i} | Vertical: ${getVerticalPosition(el)}`;
}

function relativeVisibleIndex(di) {
  const i = getVisibleIndex() + di;
  setVisibleIndex(((((i - 1) % MAXPAGE) + MAXPAGE) % MAXPAGE) + 1);
}

function ensureSection(index) {
  if (getSectionByIndex(index)) return;

  if (loading.has(index)) return;
  loading.add(index);

  const sec = document.getElementById("table-section-template").content.cloneNode(true).firstElementChild;

  sec.addEventListener('scroll', () => {
    ind.textContent = `Horizontal: Table ${getVisibleIndex()} | Vertical: ${getVerticalPosition(sec)}`;
  });
  sec.dataset = [];
  sec.dataset.index = index;
  sec.id = `sec${index}`
  const insertPos = getIndicesInDomOrder().findIndex(idx => idx > index);
  if (insertPos === -1) {
    hc.appendChild(sec);
  } else {
    const before = hc.children[insertPos];
    hc.insertBefore(sec, before);
    // sec.ariaBusy = true;
    try {
      fillSectionAsync(index, sec);
      // sec.ariaBusy = false;
    } finally {
      loading.delete(index);
    }
  }
}

function cleanupFar(currentIndex) {
  const ch = Array.from(hc.children);
  if (ch.length > 2 * KEEP_RANGE) {
    const a = Math.max(0, currentIndex - KEEP_RANGE);
    const b = Math.min(ch.length, currentIndex + KEEP_RANGE)
    hc.replaceChildren(...ch.slice(a, b));
  }
}

function preloadAround(center) {
  for (let i = center - PRELOAD_RANGE; i <= center + PRELOAD_RANGE; i++) {
    ensureSection(i);
  }
}

function getVerticalPosition(activeSec) {
  const s = activeSec || (() => {
    // Fallback locate visible section
    for (let el of document.querySelectorAll('.table-section')) {
      const rect = el.getBoundingClientRect();
      if (rect.left >= 0 && rect.left < window.innerWidth) return el;
    }
    return null;
  })();

  if (!s) return 'Top';
  const vPos = s.scrollTop;
  if (vPos < 100) return 'Top';
  if (vPos > s.scrollHeight - s.clientHeight - 100) return 'Bottom';
  return 'Middle';
}

// Scroll handling (debounced to detect snap settle)
let scrollTimer = null;
function onScroll() {
  const current = getVisibleIndex();
  ind.textContent = `Horizontal: Table ${current} | Vertical: ${getVerticalPosition(getSectionByIndex(current))}`;

  if (scrollTimer) clearTimeout(scrollTimer);
  scrollTimer = setTimeout(() => {
    const cur = getVisibleIndex();
    // preloadAround(cur);
    // cleanupFar(cur);
  }, 150);
}

hc.addEventListener('scroll', onScroll);

// Initial content: start with tables 1..3 as in the original demo
(function init() {
  // Initial preload and indicator after layout
  requestAnimationFrame(() => {
    const cur = getVisibleIndex();
    // preloadAround(cur);
    ind.textContent = `Horizontal: Table ${cur} | Vertical: ${getVerticalPosition(getSectionByIndex(cur))}`;
  });
})();
