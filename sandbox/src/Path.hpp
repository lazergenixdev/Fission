#include <Fission/Core/Renderer_2D.hh>
#include <Fission/Platform/utils.h>
#include <vector>
#include <iostream>
using namespace fs;

struct Spline {
	static constexpr int resolution = 32;

	v2f32 a, b, c;

	v2f32 point(float t) const;
	void calc_lengths(float& start, float* out) const;

	void add(Renderer_2D& r2d) const;
};

struct Path {
	std::vector<v2f32> points;
	std::vector<float> lengths;
	float total_length = 0;

	constexpr Path() = default;

	auto splines            () -> iterable<Spline>;
	void calc_total_length  ();
	auto position           (float offset) -> v2f32;
	void resolve            (rf32 bounds);
	void add_point          (v2f32 const& p);
    
	int count() const { return (int)points.size(); }
	v2f32* data() { return points.data(); }

	void add(Renderer_2D& r2d, bool control_lines, bool points);

	void save(std::ostream& os) const;
	void load(std::istream& is);
};