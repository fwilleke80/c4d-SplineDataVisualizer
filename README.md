# c4d-SplineDataSpline
A spline generator object that visualizes a SplineData

## Attributes
### SplineData
The spline curve.

### Width, Height
Dimensions of the generated spline.

### Subdivisions
The number of subdivisions in the generated spline. The SplineData is sampled at each subdivision point.

## Known bugs
* Something is wrong with the generated spline. It behaves and works as expected, but when it's converted with `c`, it does not display any points in **Point Mode**.
* Some of the standard `SplineObject` parameters do not seem to work.

## License
Published unter GPL 3.0

## Contributors
* Frank Willeke (https://github.com/fwilleke80)
