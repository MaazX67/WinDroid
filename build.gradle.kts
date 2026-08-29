android {
    namespace = "com.maazx.windroid"
    compileSdk = 34

    defaultConfig {
        minSdk = 24
        targetSdk = 34
    }

    compileOptions {
        sourceCompatibility JavaVersion.VERSION_17  // <-- YE 17 HONA CHAHIYE
        targetCompatibility JavaVersion.VERSION_17  // <-- YE BHI 17
    }
    kotlinOptions {
        jvmTarget = "17"  // <-- YE BHI 17
    }
}

dependencies {
    // Compose BOM version
    implementation(platform("androidx.compose:compose-bom:2024.09.00")) 
}

plugins {
    id("org.jetbrains.kotlin.android") version "2.0.20" // <-- YE VERSION
}
