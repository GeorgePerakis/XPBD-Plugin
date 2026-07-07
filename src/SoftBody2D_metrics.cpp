#include "SoftBody2D.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <cstdlib>

using namespace godot;

void SoftBody2D::start_metrics_capture(double duration_s, double sample_ms) {
    capture_active = true;
    capture_duration_s = duration_s > 0.0 ? duration_s : 10.0;
    sampling_interval_ms = sample_ms > 0.0 ? sample_ms : 100.0;
    capture_elapsed_ms = 0.0;
    sample_accumulator_ms = 0.0;
    metric_samples.clear();
    UtilityFunctions::print("[XPBD Metrics] Starting capture for ", capture_duration_s, "s, sampling ", sampling_interval_ms, "ms");
}

void SoftBody2D::process_metrics_capture(double delta_ms) {
    if (!capture_active) return;

    capture_elapsed_ms += delta_ms;
    sample_accumulator_ms += delta_ms;

    if (sample_accumulator_ms >= sampling_interval_ms) {
        MetricSample s;
        s.time_s = capture_elapsed_ms / 1000.0;
        s.step_ms = metric_step_time_ms;
        s.avg_ms = metric_avg_step_time_ms;
        s.max_ms = metric_max_step_time_ms;
        s.error = metric_constraint_error;
        s.particles = metric_particle_count;
        s.constraints = metric_constraint_count;
        metric_samples.push_back(s);
        sample_accumulator_ms -= sampling_interval_ms;
    }

    if (capture_elapsed_ms >= capture_duration_s * 1000.0) {
        capture_active = false;
        write_metrics_csv();
    }
}

void SoftBody2D::write_metrics_csv() {
    auto now = std::chrono::system_clock::now();
    std::time_t tnow = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&tnow);
    std::ostringstream dirname;
    dirname << "metrics/metrics_" << std::put_time(&tm, "%Y%m%d_%H%M%S");

    std::error_code ec;
    std::filesystem::create_directories(dirname.str(), ec);
    if (ec) {
        UtilityFunctions::print("[XPBD Metrics] Failed to create metrics directory");
    }

    std::ostringstream fname;
    fname << dirname.str() << "/metrics.csv";

    std::ofstream out(fname.str());
    if (!out.is_open()) {
        UtilityFunctions::print("[XPBD Metrics] Failed to open CSV file: ", fname.str().c_str());
        return;
    }

    // Write metadata header as comments
    out << "# solver_mode=" << (solver_mode == SOLVER_HIERARCHICAL ? "hierarchical" : "flat_xpbd") << "\n";
    out << "# particles=" << metric_particle_count << "\n";
    out << "# constraints=" << metric_constraint_count << "\n";
    out << "# substeps=" << num_substeps << "\n";
    out << "# grid=" << grid_width << "x" << grid_height << "\n";
    out << "# stiffness=" << grid_stiffness << "\n";
    out << "time_s,step_time_ms,avg_ms,max_ms,error,particles,constraints\n";
    out << std::fixed << std::setprecision(6);
    for (auto &s : metric_samples) {
        out << s.time_s << "," << s.step_ms << "," << s.avg_ms << "," << s.max_ms << "," << s.error << "," << s.particles << "," << s.constraints << "\n";
    }
    out.close();

    UtilityFunctions::print("[XPBD Metrics] Capture complete. CSV written to: ", fname.str().c_str());

    // Launch per-metric comparison graph generation
    std::ostringstream cmd;
#ifdef _WIN32
    cmd << "py ../tools/compare_solvers.py " << "metrics";
#else
    cmd << "python3 ../tools/compare_solvers.py " << "metrics";
#endif
    int rc = std::system(cmd.str().c_str());
    if (rc != 0) {
        UtilityFunctions::print("[XPBD Metrics] Compare script returned rc=", rc);
    }
}
