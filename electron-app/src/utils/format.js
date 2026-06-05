export function formatNumber(value, digits = 2) {
  if (value === null || value === undefined || Number.isNaN(value)) return "—";
  if (typeof value !== "number") return String(value);

  return new Intl.NumberFormat("en-US", {
    maximumFractionDigits: digits
  }).format(value);
}

export function formatInteger(value) {
  if (value === null || value === undefined || Number.isNaN(value)) return "—";

  return new Intl.NumberFormat("en-US", {
    maximumFractionDigits: 0
  }).format(value);
}

export function formatMbps(value) {
  if (value === null || value === undefined || Number.isNaN(value)) return "—";
  return `${formatNumber(value, 3)} Mbps`;
}

export function formatMs(value) {
  if (value === null || value === undefined || Number.isNaN(value)) return "—";
  return `${formatNumber(value, 2)} ms`;
}

export function formatPercent(value) {
  if (value === null || value === undefined || Number.isNaN(value)) return "—";
  return `${formatNumber(value * 100, 2)}%`;
}

export function formatBytes(value) {
  if (value === null || value === undefined || Number.isNaN(value)) return "—";
  return `${formatInteger(value)} bytes`;
}

export function lastNonEmptyLine(chunks) {
  const text = chunks.join("");
  const lines = text
    .split(/\r?\n/)
    .map((line) => line.trim())
    .filter(Boolean);

  return lines.length > 0 ? lines[lines.length - 1] : "Ready";
}
