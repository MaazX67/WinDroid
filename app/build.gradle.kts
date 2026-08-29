plugins { id("com.android.application"); id("org.jetbrains.kotlin.android") }
android {
    namespace="com.windroid"; compileSdk=34
    defaultConfig { applicationId="com.windroid"; minSdk=26; targetSdk=34; versionCode=3; versionName="3.0-Native" }
    buildTypes { release { isMinifyEnabled=false } }
    compileOptions { sourceCompatibility=JavaVersion.VERSION_1_8; targetCompatibility=JavaVersion.VERSION_1_8 }
    externalNativeBuild { cmake { path = file("src/main/cpp/CMakeLists.txt") } }
}
dependencies { implementation("androidx.core:core-ktx:1.12.0") }
