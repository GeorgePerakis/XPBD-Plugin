#!/usr/bin/env python3
"""
Auto-compare the latest flat_xpbd and hierarchical captures.
Finds the most recent CSV for each solver mode in the metrics/ directory,
generates per-metric comparison graphs, and saves them as PNGs.

Usage:
  python tools/compare_solvers.py [metrics_dir]

Default metrics_dir: metrics/
Outputs PNG files into metrics/comparison_latest/
"""
import sys
import os
import csv
import glob
import matplotlib.pyplot as plt
from pathlib import Path


def parse_csv(csv_path):
    """Parse a metrics CSV, extracting metadata comments and data rows."""
    metadata = {}
    times, step_ms, avg_ms, max_ms, errors, particles, constraints = [], [], [], [], [], [], []

    with open(csv_path, newline='') as f:
        lines = f.readlines()

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


def find_latest_by_mode(metrics_dir):
    """Find the latest CSV for each solver mode."""
    csv_files = sorted(glob.glob(os.path.join(metrics_dir, "metrics_*", "metrics.csv")))

    latest = {}  # solver_mode -> (path, mtime)
    for csv_path in csv_files:
        meta, *_ = parse_csv(csv_path)
        mode = meta.get('solver_mode', 'unknown')
        mtime = os.path.getmtime(csv_path)
        if mode not in latest or mtime > latest[mode][1]:
            latest[mode] = (csv_path, mtime)

    return {mode: path for mode, (path, _) in latest.items()}


def make_label(metadata):
    solver = metadata.get('solver_mode', 'unknown')
    grid = metadata.get('grid', '?')
    substeps = metadata.get('substeps', '?')
    return f"{solver} ({grid}, sub={substeps})"


def generate_comparison(metrics_dir):
    latest = find_latest_by_mode(metrics_dir)

    if 'flat_xpbd' not in latest:
        print("No flat_xpbd capture found in", metrics_dir)
        return
    if 'hierarchical' not in latest:
        print("No hierarchical capture found in", metrics_dir)
        return

    print(f"Flat XPBD:     {latest['flat_xpbd']}")
    print(f"Hierarchical:  {latest['hierarchical']}")

    # Parse both
    flat_meta, flat_t, flat_step, flat_avg, flat_max, flat_err, flat_p, flat_c = parse_csv(latest['flat_xpbd'])
    hier_meta, hier_t, hier_step, hier_avg, hier_max, hier_err, hier_p, hier_c = parse_csv(latest['hierarchical'])

    flat_label = make_label(flat_meta)
    hier_label = make_label(hier_meta)

    # Output directory
    out_dir = os.path.join(metrics_dir, "comparison_latest")
    os.makedirs(out_dir, exist_ok=True)

    colors = {'flat': '#1f77b4', 'hier': '#ff7f0e'}

    # --- 1. Step Time (instantaneous) ---
    fig, ax = plt.subplots(figsize=(10, 5))
    ax.plot(flat_t, flat_step, color=colors['flat'], alpha=0.6, linewidth=1, label=flat_label)
    ax.plot(hier_t, hier_step, color=colors['hier'], alpha=0.6, linewidth=1, label=hier_label)
    ax.set_title('Step Time (per frame)')
    ax.set_xlabel('time (s)')
    ax.set_ylabel('step_time (ms)')
    ax.legend()
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    fig.savefig(os.path.join(out_dir, "step_time.png"), dpi=150)
    plt.close(fig)

    # --- 2. Average Step Time ---
    fig, ax = plt.subplots(figsize=(10, 5))
    ax.plot(flat_t, flat_avg, color=colors['flat'], linewidth=2, label=flat_label)
    ax.plot(hier_t, hier_avg, color=colors['hier'], linewidth=2, label=hier_label)
    ax.set_title('Average Step Time (running)')
    ax.set_xlabel('time (s)')
    ax.set_ylabel('avg_ms')
    ax.legend()
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    fig.savefig(os.path.join(out_dir, "avg_step_time.png"), dpi=150)
    plt.close(fig)

    # --- 3. Max Step Time ---
    fig, ax = plt.subplots(figsize=(10, 5))
    ax.plot(flat_t, flat_max, color=colors['flat'], linewidth=2, label=flat_label)
    ax.plot(hier_t, hier_max, color=colors['hier'], linewidth=2, label=hier_label)
    ax.set_title('Max Step Time (worst case)')
    ax.set_xlabel('time (s)')
    ax.set_ylabel('max_ms')
    ax.legend()
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    fig.savefig(os.path.join(out_dir, "max_step_time.png"), dpi=150)
    plt.close(fig)

    # --- 4. Constraint Error ---
    fig, ax = plt.subplots(figsize=(10, 5))
    ax.plot(flat_t, flat_err, color=colors['flat'], linewidth=2, label=flat_label)
    ax.plot(hier_t, hier_err, color=colors['hier'], linewidth=2, label=hier_label)
    ax.set_title('Constraint Error (total residual)')
    ax.set_xlabel('time (s)')
    ax.set_ylabel('error')
    ax.legend()
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    fig.savefig(os.path.join(out_dir, "constraint_error.png"), dpi=150)
    plt.close(fig)

    # --- 5. Combined overview (all metrics in subplots) ---
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))

    ax = axes[0, 0]
    ax.plot(flat_t, flat_step, color=colors['flat'], alpha=0.5, linewidth=0.8, label=flat_label)
    ax.plot(hier_t, hier_step, color=colors['hier'], alpha=0.5, linewidth=0.8, label=hier_label)
    ax.set_title('Step Time (per frame)')
    ax.set_ylabel('ms')
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.3)

    ax = axes[0, 1]
    ax.plot(flat_t, flat_avg, color=colors['flat'], linewidth=2, label=flat_label)
    ax.plot(hier_t, hier_avg, color=colors['hier'], linewidth=2, label=hier_label)
    ax.set_title('Average Step Time')
    ax.set_ylabel('ms')
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.3)

    ax = axes[1, 0]
    ax.plot(flat_t, flat_max, color=colors['flat'], linewidth=2, label=flat_label)
    ax.plot(hier_t, hier_max, color=colors['hier'], linewidth=2, label=hier_label)
    ax.set_title('Max Step Time')
    ax.set_xlabel('time (s)')
    ax.set_ylabel('ms')
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.3)

    ax = axes[1, 1]
    ax.plot(flat_t, flat_err, color=colors['flat'], linewidth=2, label=flat_label)
    ax.plot(hier_t, hier_err, color=colors['hier'], linewidth=2, label=hier_label)
    ax.set_title('Constraint Error')
    ax.set_xlabel('time (s)')
    ax.set_ylabel('error')
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.3)

    fig.suptitle('XPBD Solver Comparison: Flat vs Hierarchical', fontsize=14, fontweight='bold')
    fig.tight_layout()
    fig.savefig(os.path.join(out_dir, "comparison_overview.png"), dpi=150)
    plt.close(fig)

    # --- Print summary ---
    print(f"\n{'='*60}")
    print(f"{'Metric':<25} {'Flat XPBD':>15} {'Hierarchical':>15}")
    print(f"{'-'*60}")
    print(f"{'Final avg_ms':<25} {flat_avg[-1]:>15.4f} {hier_avg[-1]:>15.4f}")
    print(f"{'Final max_ms':<25} {flat_max[-1]:>15.4f} {hier_max[-1]:>15.4f}")
    print(f"{'Final error':<25} {flat_err[-1]:>15.4f} {hier_err[-1]:>15.4f}")
    print(f"{'Particles':<25} {flat_p[-1]:>15} {hier_p[-1]:>15}")
    print(f"{'Constraints':<25} {flat_c[-1]:>15} {hier_c[-1]:>15}")
    print(f"{'='*60}")
    print(f"\nGraphs saved to: {out_dir}/")
    print(f"  - step_time.png")
    print(f"  - avg_step_time.png")
    print(f"  - max_step_time.png")
    print(f"  - constraint_error.png")
    print(f"  - comparison_overview.png")


if __name__ == '__main__':
    metrics_dir = sys.argv[1] if len(sys.argv) > 1 else 'metrics'
    generate_comparison(metrics_dir)
