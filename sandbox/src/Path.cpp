#include "Path.hpp"

v2f32 Spline::point(float t) const {
    v2f32 const u = a + (b - a) * t;
    v2f32 const v = b + (c - b) * t;
    return u + (v - u) * t;
}

void Spline::calc_lengths(float& start, float* out) const {
	v2f32 prev = point(0);
	float const dt = 1.0f / float(resolution);
	--out;
	for (int i = 1; i <= resolution; ++i) {
		auto const p = point(float(i) * dt);
		float const length = (prev - p).len();
		out[i] = start + length;
		start += length;
		prev = p;
	}
}

void Spline::add(Renderer_2D& r2d) const {
    v2f32 prev = point(0);
    float const dt = 1.0f / float(resolution);
    for (int i = 1; i <= resolution; ++i) {
        auto const p = point(float(i)*dt);
        //auto const col = i&1? colors::Red : colors::Blue;
        auto const col = colors::White;
        r2d.add_line(prev, p, 1.0f, col, col);
        prev = p;
    }
}


iterable<Spline> Path::splines() {
	return iterable<Spline>{ points.data(), (u32)points.size() / 2, sizeof(v2f32) * 2 };
}

void Path::calc_total_length() {
	int const ns = (int)points.size() / 2;
	lengths.resize(ns * Spline::resolution);
	total_length = 0;
	for (int i = 0; i < ns; ++i) {
		auto s = reinterpret_cast<Spline const*>(points.data() + i * 2);
		s->calc_lengths(total_length, lengths.data() + i * Spline::resolution);
	}
}

v2f32 Path::position(float offset) {
	// binary search over lengths
	v2f32 const* s = points.data();
	float t;
	if (offset < lengths[0]) {
		t = (offset / lengths[0]) / float(Spline::resolution);
	}
	else {
		int a = 0, b = (int)lengths.size() - 1;
		while (b - a > 1) {
			int const m = (a + b) / 2;
			float const l = lengths[m];
			if (l < offset) a = m;
			else            b = m;
		}
		int const i = b / Spline::resolution;
		t = ((offset - lengths[a]) / (lengths[b] - lengths[a])) / float(Spline::resolution);
		t += float(b - i * Spline::resolution) / float(Spline::resolution);
		s += i * 2;
	}

	// find point on spline
	return reinterpret_cast<Spline const*>(s)->point(t);
}

void Path::resolve(rf32 bounds) {
	int n = (int)points.size();
	FS_FOR(n) points[i] = bounds.clamp(points[i]);
	for (int i = 2; i < n - 1; i += 2) {
		v2f32 const a = points[i - 1];
		v2f32 const p = points[i];
		v2f32 const b = points[i + 1];
		v2f32 const p0 = p - a;
		v2f32 const d = b - a;
		float const h = dot(p0, d.norm());
		points[i] = d.norm() * h + a;
	}
}

void Path::add_point(v2f32 const& p) {
	if (!points.size()) {
		points.emplace_back(p);
		return;
	}

	points.emplace_back(lerp(points[points.size()-2], points.back(), 2.0f));
	points.emplace_back(p);
}

void Path::add(Renderer_2D& r2d, bool control_lines, bool show_points) {
	auto ss = splines();
	for (auto const& s : ss) s.add(r2d);
	if (control_lines)
	for (int i = 0; i < (int)ss.count - 1; ++i) {
		v2f32 const a = points[i * 2 + 1];
		v2f32 const b = points[i * 2 + 3];
		r2d.add_line(a, b, 1.0f, colors::Green, colors::Green);
	}
	if (show_points)
	for (auto const& p : points) {
		r2d.add_circle(p, 6.0f, colors::Blue);
	}
}

#define WRITE(OBJ) write((char const*)(&OBJ), sizeof(OBJ))
#define  READ(OBJ) read (      (char*)(&OBJ), sizeof(OBJ))

void Path::save(std::ostream& os) const {
    u32 count = (u32)points.size();
    os.WRITE(count);
    os.write((char const*)points.data(), sizeof(v2f32) * points.size());
}

void Path::load(std::istream& is) {
    u32 count;
    is.READ(count);
	points.clear();
    points.resize(count);
    is.read((char*)points.data(), count*sizeof(v2f32));
	calc_total_length();
}
