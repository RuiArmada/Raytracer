# Physically-based rendering

https://typhomnt.github.io/teaching/ray_tracing/pbr_intro/

At the moment, our _renderer_ does not conserve energy. A surface can reflect more light than it receives, resulting in unrealistic _renders_ and frequent _overexposure_.

We can solve this problem by using a PBR (Physically Based Rendering) model.

## Micro-facet

In the PBR model, each surface is simulated as being composed of micro-faces, each pointing in a slightly different direction. The smoother the surface, the more uniform the direction of these micro-faces, being completely uniform in a mirror.

## Light Equation
The light at a given point $p$, as seen from the direction $v$, is given by the equation:

$$L_o(p, v) = \int_{A} f_r(p, l, v, \alpha_p) L_i(p, l)(n * l) dl$$

- $l$ -> Direction of incident light
- $\alpha_p$ -> _roughness_ of the surface at point $p$
- $L_i$ -> Radiance of the incident light (RGB color)
- $f_r$ -> Function that controls the reflected light in the material
- $A$ -> Hemisphere around point $p$

## Notes

`illum` specifies the lighting model to use. If it is 3, it is metallic, and the "Metallic" parameter is represented in the argument Ka

http://paulbourke.net/dataformats/mtl/
