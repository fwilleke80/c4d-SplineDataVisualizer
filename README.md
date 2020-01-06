# SplineDataVisualizer
Render SplineData curves!

## Components

### SplineData Spline Object
A Spline Object that visualizes data from a SplineData GUI gadget.

#### SplineData
The spline curve.

#### Width, Height
Dimensions of the generated spline.

#### Subdivisions
The number of subdivisions in the generated spline. The SplineData is sampled at each subdivision point.

### SplineData Axis Spline Object
A Spline Object that displays a 2-axis coordinate system with optional arrows

#### Width, Height
Dimensions of the generated system.

#### Overshoot
Make axes overshoot the origin point.

#### Draw Arrows
Enable to draw arrows.

#### Arrow Width, Arrow Height
Dimensions of the generated arrows.

#### Arrow Overshoot
Move arrows along axis ends, axis will adapt automatically.

### Face Camera Tag
A simple tag that will inherit the active camera's alignment and apply it to the object it's attached to, while preserving the position and scale of the object. Long story short, the object carrying the tag will always face the camera, but can be freely moved and scaled in the viewport.

### Create SplineData Visualization Command
One command to build a complete setup with SplineData Spline, Axis Spline, and text. Ready to render.

## Known bugs
* Some of the standard `SplineObject` parameters do not seem to work as expected.

## License
Published unter GPL 3.0

## Contributors
* Frank Willeke (https://github.com/fwilleke80)
