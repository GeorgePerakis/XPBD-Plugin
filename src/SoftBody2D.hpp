#pragma once

#include "godot_cpp/classes/node2d.hpp"
#include "godot_cpp/classes/sprite2d.hpp"
#include "godot_cpp/classes/texture2d.hpp"
#include "structs/Distance_Constraint.hpp"
#include "structs/Particle.hpp"
#include <vector>

enum SolverMode {
    SOLVER_FLAT_XPBD = 0,
    SOLVER_HIERARCHICAL = 1,
};

class SoftBody2D: public godot::Node2D {
    GDCLASS(SoftBody2D, godot::Node2D)

public:
	void _physics_process(double delta) override;
    void _ready() override;
    void _draw() override;

protected:
	static void _bind_methods();

private:
    godot::Ref<godot::Texture2D> particle_texture;
    
    std::vector<Particle> Particles;
    std::vector<godot::Sprite2D*> particle_sprites;
    std::vector<Distance_Constraint> Distance_Constraints;
    double floor_y = 1080.0;
    int num_substeps = 10;
    int solver_iterations = 1;

    // Solver mode
    int solver_mode = SOLVER_FLAT_XPBD;

    // Hierarchical priority groups (indices into Distance_Constraints)
    std::vector<std::vector<int>> priority_groups;
    void build_priority_groups();

    // Stress test parameters
    int grid_width = 2;
    int grid_height = 2;
    double grid_spacing = 50.0;
    double grid_mass = 1.0;
    double grid_stiffness = 100.0;

    // Metrics
    double metric_step_time_ms = 0.0;
    double metric_constraint_error = 0.0;
    int metric_particle_count = 0;
    int metric_constraint_count = 0;
    int metric_frame_count = 0;
    double metric_avg_step_time_ms = 0.0;
    double metric_max_step_time_ms = 0.0;
    double metric_total_step_time_ms = 0.0;

    struct MetricSample {
        double time_s = 0.0;
        double step_ms = 0.0;
        double avg_ms = 0.0;
        double max_ms = 0.0;
        double error = 0.0;
        int particles = 0;
        int constraints = 0;
    };

    // Capture state
    bool auto_start_capture = true;
    bool capture_active = false;
    double capture_duration_s = 10.0;
    double sampling_interval_ms = 100.0;
    double capture_elapsed_ms = 0.0;
    double sample_accumulator_ms = 0.0;
    std::vector<MetricSample> metric_samples;

    void start_metrics_capture(double duration_s = 10.0, double sample_ms = 100.0);
    void process_metrics_capture(double delta_ms);
    void write_metrics_csv();

    void spawn_grid(int width, int height, double spacing, double mass, double stiffness);

    void step(double delta);
    void solve_flat(double sub_dt);
    void solve_hierarchical(double sub_dt);
    double compute_constraint_error();

    void create_particle_sprites();

    void set_particle_texture(const godot::Ref<godot::Texture2D> &tex) { particle_texture = tex; }
    godot::Ref<godot::Texture2D> get_particle_texture() const { return particle_texture; }

    void set_num_substeps(int value) { num_substeps = value > 0 ? value : 1; }
    int get_num_substeps() const { return num_substeps; }

    void set_solver_iterations(int value) { solver_iterations = value > 0 ? value : 1; }
    int get_solver_iterations() const { return solver_iterations; }

    void set_grid_width(int value) { grid_width = value > 1 ? value : 2; }
    int get_grid_width() const { return grid_width; }

    void set_grid_height(int value) { grid_height = value > 1 ? value : 2; }
    int get_grid_height() const { return grid_height; }

    void set_grid_spacing(double value) { grid_spacing = value; }
    double get_grid_spacing() const { return grid_spacing; }

    void set_grid_mass(double value) { grid_mass = value > 0.0 ? value : 1.0; }
    double get_grid_mass() const { return grid_mass; }

    void set_grid_stiffness(double value) { grid_stiffness = value > 0.0 ? value : 1.0; }
    double get_grid_stiffness() const { return grid_stiffness; }

    double get_metric_step_time_ms() const { return metric_step_time_ms; }
    double get_metric_constraint_error() const { return metric_constraint_error; }
    int get_metric_particle_count() const { return metric_particle_count; }
    int get_metric_constraint_count() const { return metric_constraint_count; }
    double get_metric_avg_step_time_ms() const { return metric_avg_step_time_ms; }
    double get_metric_max_step_time_ms() const { return metric_max_step_time_ms; }

    void set_auto_start_capture(bool v) { auto_start_capture = v; }
    bool get_auto_start_capture() const { return auto_start_capture; }

    void set_capture_duration(double v) { capture_duration_s = v > 0.0 ? v : 10.0; }
    double get_capture_duration() const { return capture_duration_s; }

    void set_sampling_interval(double v) { sampling_interval_ms = v > 0.0 ? v : 100.0; }
    double get_sampling_interval() const { return sampling_interval_ms; }

    void set_solver_mode(int v) { solver_mode = (v == SOLVER_HIERARCHICAL) ? SOLVER_HIERARCHICAL : SOLVER_FLAT_XPBD; }
    int get_solver_mode() const { return solver_mode; }
};
