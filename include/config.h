/*
static const char *background_color = "#3e3e3e";
static const char *border_color = "#ececec";
static const char *font_color = "#ececec";
static const char *font_pattern = "monospace:size=10";
static const unsigned line_spacing = 5;
static const unsigned int padding = 15;
*/

//Style options
static int32_t margin_right = 5, margin_bottom = 0, margin_left = 0, margin_top = 5;
static uint32_t anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP + ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT; 
static uint32_t width = 450;
static const unsigned int border_size = 3;
static double alpha = 0.9;
static float font_size = 16.0;

//These all accept a value of 0.0 to 1.0 for red, green, and blue. The last character corresponds to the color
//Background color
static const float bgr = 0.2;
static const float bgb = 0.2;
static const float bgg = 0.2;

//Border color
static const float brr = 1.0;
static const float brb = 1.0;
static const float brg = 1.0;

//Font color
static const float fr = 1.0;
static const float fb = 1.0;
static const float fg = 1.0;

static const unsigned int duration = 5;
