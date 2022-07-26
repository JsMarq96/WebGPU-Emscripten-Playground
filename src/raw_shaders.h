#ifndef RAW_SHADERS_H_
#define RAW_SHADERS_H_

const char raw_basic_shader[] = R"(
struct sVertOut {
    @builtin(position) Position: vec4<f32>,
    @location(0) inColor: vec3<f32>,
};

@vertex
fn vert_main(@location(0) inPos: vec3<f32>,
             @location(1) inColor: vec3<f32>) -> sVertOut {
    var out: sVertOut;
    out.Position = vec4<f32>(inPos, 1.0);
    out.inColor = inColor;
    return out;
}

@fragment
fn frag_main(@location(0) inColor: vec3<f32>) -> @location(0) vec4<f32> {
    return vec4<f32>(1.0, 0.0, 0.0, 1.0);
}
)";

const char raw_basic_shader_vert[] = R"(
struct sVertOut {
    @builtin(position) Position: vec4<f32>,
    @location(1) color: vec3<f32>,
};

@vertex
fn main( inPos: vec3<f32>,
        @location(1) inColor: vec3<f32>) -> sVertOut {
    var out: sVertOut;
    out.Position = vec4<f32>(inPos, 1.0);
    out.color = inColor;
    return out;
}
)";

const char raw_basic_shader_frag[] = R"(
@fragment
fn main(@location(0) inColor: vec3<f32>) -> @location(0) vec4<f32> {
    return vec4<f32>(0.0, 1.0, 0.0, 1.0);
}
)";


#endif // RAW_SHADERS_H_
