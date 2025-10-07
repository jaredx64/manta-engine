#include <labels.hpp>

#include <manta/window.hpp>
#include <manta/draw.hpp>
#include <manta/vector.hpp>
#include <manta/3d.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <earth.hpp>
#include <view.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Location { double latitude; double longitude; const char *name; };

static const Location locations[] =
{
	{ 35.5147457, 139.4839981, "Tokyo" },
	{ 14.5906216, 120.9799696, "Manila" },
	{ -6.1753942, 106.827183, "Jakarta" },
	{ 28.6517178, 77.2219388, "Delhi" },
	{ 37.5056926, 126.7358775, "Seoul" },
	{ 18.9387711, 72.8353355, "Mumbai" },
	{ 19.4326296, -99.1331785, "Mexico City" },
	{ -18.9137, 47.5361, "Antananarivo" },
	{ 40.7127281, -74.0060152, "New York" },
	{ 14.7, -17.5, "Dakar" },
	{ -16.5004, -151.7415, "Bora Bora" },
	{ 55.7504461, 37.6174943, "Moscow" },
	{ 30.048819, 31.243666, "Cairo" },
	{ 39.906217, 116.3912757, "Beijing" },
	{ 24.8667795, 67.0311286, "Karachi" },
	{ -34.6075682, -58.4370894, "Buenos Aires" },
	{ 34.0536909, -118.2427666, "Los Angeles" },
	{ 13.7542529, 100.493087, "Bangkok" },
	{ -13.005, -38.5177, "Salvador" },
	{ 6.4550575, 3.3941795, "Lagos" },
	{ 43.25, 76.9, "Almaty" },
	{ -31.95, 115.86, "Perth" },
	{ 41.0096334, 28.9651646, "Istanbul" },
	{ 35.7006177, 51.4013785, "Tehran" },
	{ 56.2928, 101.7121, "Bratsk" },
	{ 31.2252985, 121.4890497, "Shanghai" },
	{ -22.9110137, -43.2093727, "Rio de Janeiro" },
	{ -12.0621065, -77.0365256, "Lima" },
	{ 31.5656079, 74.3141775, "Lahore" },
	{ 10.6497452, 106.7619794, "Ho Chi Minh City" },
	{ 51.5073219, -0.1276474, "London" },
	{ 12.9791198, 77.5912997, "Bangalore" },
	{ 48.8566969, 2.3514616, "Paris" },
	{ 30.6624205, 104.0633219, "Chengdu" },
	{ 21.35, -157.93, "Honolulu" },
	{ 21.15, 79.09, "Nagpur" },
	{ 51.169392, 71.4467, "Astana" },
	{ 4.59808, -74.0760439, "Bogotá" },
	{ 24.9929995, 121.3010003, "Taipei" },
	{ -4.3217055, 15.3125974, "Kinshasa" },
	{ 41.8755616, -87.6244212, "Chicago" },
	{ 30.5951051, 114.2999353, "Wuhan" },
	{ -33.55, 18.25, "Cape Town" },
	{ -33.4489, -70.6693, "Santiago" },
	{ 7.196276, 125.461807, "Davao" },
	{ 21.0294498, 105.8544441, "Hanoi" },
	{ 23.0216238, 72.5797068, "Ahmedabad" },
	{ 61.22, -149.9, "Anchorage" },
	{ -8.8272699, 13.2439512, "Luanda" },
	{ 33.3024309, 44.3787992, "Baghdad" },
	{ 34.3075655, 108.7234362, "Xi'an" },
	{ 22.2793278, 114.1628131, "Hong Kong" },
	{ 39.742043, -104.991531, "Denver" },
	{ 33.8869, 9.5375, "Tunis" },
	{ 38.63, -90.20, "St. Louis" },
	{ -25.7566, 28.1914, "Pretoria" },
	{ -17.7134, 178.0650, "Fiji" },
	{ 43.6706177, -79.3746817, "Toronto" },
	{ -43.525650, 172.639847, "Christchurch" },
	{ 32.7379037, -97.2394281, "Dallas" },
	{ 9.8694792, -83.7980749, "Santiago" },
	{ 5.85, -55.20, "Paramaribo" },
	{ 52.5243, 13.4063, "Berlin" },
	{ 40.4167047, -3.7035825, "Madrid" },
	{ 21.5292, 39.1611, "Jeddah" },
	{ 25.276987, 55.296249, "Dubai" },
	{ 15.356695, 44.200218, "Sanaa" },
	{ -1.2832533, 36.8172449, "Nairobi" },
	{ 36.4660932, 120.6190022, "Qingdao" },
	{ 25.7742658, -80.1936589, "Miami" },
	{ 20.95, -89.65, "Merida" },
	{ -9.4790, 147.1494, "Port Moresby" },
	{ 15.593325, 32.53565, "Khartoum" },
	{ 31.9515694, 35.9239625, "Amman" },
	{ 33.7490987, -84.3901849, "Atlanta" },
	{ 1.340863, 103.8303918, "Singapore" },
	{ 62.035454, 129.675476, "Yakutsk" },
	{ 16.7967129, 96.1609916, "Yangon" },
	{ 24.8797, 102.8332, "Kunming" },
	{ 45.4668, 9.1905, "Milan" },
	{ 36.19924, 37.1637253, "Aleppo" },
	{ 59.938732, 30.316229, "Saint Petersburg" },
	{ -6.8160837, 39.2803583, "Dar es Salaam" },
	{ 43.1332, 131.9113, "Vladivostok" },
	{ -25.27, -57.63, "Asunción" },
	{ 41.3828939, 2.1774322, "Barcelona" },
	{ -3.11889, -60.02167, "Manaus" },
	{ 41.9028, 12.4964, "Rome" },
	{ 29.3797091, 47.9735629, "Kuwait City" },
	{ 45.7656666, 126.6160584, "Harbin" },
	{ -33.8548157, 151.2164539, "Sydney" },
	{ 5.320357, -4.016107, "Abidjan" },
	{ 33.5950627, -7.6187768, "Casablanca" },
	{ -37.8142176, 144.9631608, "Melbourne" },
	{ 53.5462, -113.4937, "Edmonton" },
	{ 6.9349969, 79.8538463, "Colombo" },
	{ 25.6802019, -100.3152586, "Monterrey" },
	{ -7.2459717, 112.7378266, "Surabaya" },
	{ 39.9207774, 32.854067, "Ankara" },
	{ 47.61, -122.33, "Seattle" }
};
static_assert( ARRAY_LENGTH( locations ) == 100, "Missing Location!" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void labels_draw_ui( const Delta delta )
{
	const double_v3 positionEyeNorm = double_v3_normalize( View::position );
	const double_v2 screenDimensions = double_v2 { Window::width, Window::height };
	int_v2 arrowDimensions = text_dimensions( fnt_iosevka, 8, "▼" );
	int_v2 labelDimensions;

	for( int i = 0; i < 100; i++ )
	{
		const double latitude = locations[i].latitude;
		const double longitude = locations[i].longitude;
		const double altitude = 0.0;

		double_v3 pointCartesian = point_latlon_to_cartesian( latitude, longitude, altitude );
		double_v2 pointScreen = point_world_to_screen( pointCartesian, screenDimensions,
			View::matrixView, View::matrixPerspective );

		const double_v3 positionLabelNorm = double_v3_normalize( pointCartesian );
		const double positionDotEye = positionLabelNorm.dot( positionEyeNorm );
		if( positionDotEye < 0.5 ) { continue; }

		const u8 alpha = static_cast<u8>( clamp( ( positionDotEye - 0.5 ) / 0.25, 0.0, 1.0 ) * 255.0 );
		const Color colorWhite = Color { 0xFF, 0xFF, 0xFF, alpha };
		const Color colorBlack = Color { 0x33, 0x33, 0x33, alpha };

		// Arrow
		draw_text( fnt_iosevka, 8,
			pointScreen.x - arrowDimensions.x / 2 + 1,
			pointScreen.y - arrowDimensions.y, colorBlack, "▼" );
		draw_text( fnt_iosevka, 8,
			pointScreen.x - arrowDimensions.x / 2 + 0,
			pointScreen.y - arrowDimensions.y, colorWhite, "▼" );

		// Label
		labelDimensions = text_dimensions( fnt_iosevka, 15, locations[i].name );
		draw_text( fnt_iosevka, 15,
			pointScreen.x - labelDimensions.x / 2 + 1,
			pointScreen.y - labelDimensions.y - arrowDimensions.y - 4, colorBlack, locations[i].name );
		draw_text( fnt_iosevka, 15,
			pointScreen.x - labelDimensions.x / 2 + 0,
			pointScreen.y - labelDimensions.y - arrowDimensions.y - 4, colorWhite, locations[i].name );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////