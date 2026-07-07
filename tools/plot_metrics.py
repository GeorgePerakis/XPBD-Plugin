#!/usr/bin/env python3
"""
Simple script to plot metrics CSV produced by SoftBody2D capture.
Usage: python tools/plot_metrics.py metrics/metrics_YYYYMMDD_HHMMSS.csv
"""
import sys
import csv
import matplotlib.pyplot as plt

if len(sys.argv) < 2:
    print("Usage: python tools/plot_metrics.py <csv_path>")
    sys.exit(1)

csv_path = sys.argv[1]

times = []
step_ms = []
avg_ms = []
max_ms = []
errors = []
particles = []
constraints = []

with open(csv_path, newline='') as f:
    reader = csv.DictReader(f)
    for row in reader:
        times.append(float(row['time_s']))
        step_ms.append(float(row['step_time_ms']))
        avg_ms.append(float(row['avg_ms']))
        max_ms.append(float(row['max_ms']))
        errors.append(float(row['error']))
        particles.append(int(float(row['particles'])))
        constraints.append(int(float(row['constraints'])))

fig, ax1 = plt.subplots()
ax1.plot(times, step_ms, label='step_time_ms')
ax1.plot(times, avg_ms, label='avg_ms')
ax1.plot(times, max_ms, label='max_ms')
ax1.set_xlabel('time (s)')
ax1.set_ylabel('ms')

ax2 = ax1.twinx()
ax2.plot(times, errors, label='constraint_error', color='C3', linestyle='--')
ax2.set_ylabel('error')

# Plot counts in a separate subplot
fig2, ax3 = plt.subplots()
ax3.plot(times, particles, label='particles')
ax3.plot(times, constraints, label='constraints')
ax3.set_xlabel('time (s)')
ax3.set_ylabel('count')

# Legends
lines1, labels1 = ax1.get_legend_handles_labels()
lines2, labels2 = ax2.get_legend_handles_labels()
ax1.legend(lines1 + lines2, labels1 + labels2, loc='upper left')
ax3.legend()

plt.show()
