#!/usr/bin/env python3
"""
Plot metrics CSVs produced by SoftBody2D capture.
Supports overlaying multiple captures for comparison.

Usage:
  python tools/plot_metrics.py <csv1> [csv2] [csv3] ...

Each CSV can have metadata comment lines (starting with #) like:
  # solver_mode=flat_xpbd
  # particles=400
  # grid=20x20
"""
import sys
import csv
import matplotlib.pyplot as plt


def parse_csv(csv_path):
    """Parse a metrics CSV, extracting metadata comments and data rows."""
    metadata = {}
    times, step_ms, avg_ms, max_ms, errors, particles, constraints = [], [], [], [], [], [], []

    with open(csv_path, newline='') as f:
        lines = f.readlines()

    # Extract metadata from comment lines
    data_lines = []
    for line in lines:
        if line.startswith('#'):
            parts = line[1:].strip().split('=', 1)
            if len(parts) == 2:
                metadata[parts[0].strip()] = parts[1].strip()
        else:
            data_lines.append(line)

    reader = csv.DictReader(data_lines)
    for row in reader:
        times.append(float(row['time_s']))
        step_ms.append(float(row['step_time_ms']))
        avg_ms.append(float(row['avg_ms']))
        max_ms.append(float(row['max_ms']))
        errors.append(float(row['error']))
        particles.append(int(float(row['particles'])))
        constraints.append(int(float(row['constraints'])))

    return metadata, times, step_ms, avg_ms, max_ms, errors, particles, constraints


def make_label(csv_path, metadata):
    """Create a legend label from metadata or filename."""
    solver = metadata.get('solver_mode', 'unknown')
    grid = metadata.get('grid', '?')
    substeps = metadata.get('substeps', '?')
    return f"{solver} ({grid}, sub={substeps})"


if len(sys.argv) < 2:
    print("Usage: python tools/plot_metrics.py <csv1> [csv2] [csv3] ...")
    sys.exit(1)

csv_paths = sys.argv[1:]
datasets = []
for path in csv_paths:
    datasets.append((path, *parse_csv(path)))

# --- Timing plot ---
fig1, ax1 = plt.subplots(figsize=(10, 6))
ax1.set_title('Step Time Comparison')
ax1.set_xlabel('time (s)')
ax1.set_ylabel('ms')

colors = plt.cm.tab10.colors
for i, (path, meta, times, step_ms, avg_ms, max_ms, errors, particles, constraints) in enumerate(datasets):
    label = make_label(path, meta)
    c = colors[i % len(colors)]
    ax1.plot(times, step_ms, color=c, alpha=0.3, linewidth=0.8)
    ax1.plot(times, avg_ms, color=c, linewidth=2, label=f"{label} avg")
    ax1.plot(times, max_ms, color=c, linestyle='--', linewidth=1, label=f"{label} max")

ax1.legend(loc='upper left')
ax1.grid(True, alpha=0.3)

# --- Error plot ---
fig2, ax2 = plt.subplots(figsize=(10, 6))
ax2.set_title('Constraint Error Comparison')
ax2.set_xlabel('time (s)')
ax2.set_ylabel('total constraint error')

for i, (path, meta, times, step_ms, avg_ms, max_ms, errors, particles, constraints) in enumerate(datasets):
    label = make_label(path, meta)
    c = colors[i % len(colors)]
    ax2.plot(times, errors, color=c, linewidth=2, label=label)

ax2.legend(loc='upper right')
ax2.grid(True, alpha=0.3)

# --- Summary table ---
if len(datasets) > 1:
    print("\n=== Comparison Summary ===")
    print(f"{'Label':<40} {'Avg ms':>8} {'Max ms':>8} {'Final Error':>12}")
    print("-" * 72)
    for path, meta, times, step_ms, avg_ms, max_ms, errors, particles, constraints in datasets:
        label = make_label(path, meta)
        final_avg = avg_ms[-1] if avg_ms else 0
        final_max = max_ms[-1] if max_ms else 0
        final_err = errors[-1] if errors else 0
        print(f"{label:<40} {final_avg:>8.3f} {final_max:>8.3f} {final_err:>12.4f}")

plt.show()
