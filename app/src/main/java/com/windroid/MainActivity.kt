package com.windroid

import android.os.Bundle
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            MaterialTheme(colorScheme = darkColorScheme()) {
                WinDroidScreen()
            }
        }
    }
}

data class AppItem(val name: String, val icon: ImageVector, val color: Color)

@Composable
fun WinDroidScreen() {
    val context = LocalContext.current
    val pinnedApps = listOf(
        AppItem("File Explorer", Icons.Filled.Folder, Color(0xFF60A5FA)),
        AppItem("Wine", Icons.Filled.WineBar, Color(0xFFA78BFA)),
        AppItem("Box64", Icons.Filled.ViewInAr, Color(0xFFFB923C)),
        AppItem("Settings", Icons.Filled.Settings, Color(0xFF9CA3AF)),
        AppItem("Terminal", Icons.Filled.Terminal, Color(0xFF4ADE80)),
        AppItem("Notepad", Icons.Filled.Article, Color(0xFF60A5FA)),
        AppItem("Edge", Icons.Filled.Language, Color(0xFF22D3EE)),
        AppItem("Photos", Icons.Filled.PhotoLibrary, Color(0xFF60A5FA)),
        AppItem("Store", Icons.Filled.Shop, Color(0xFF93C5FD)),
    )

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(Brush.verticalGradient(listOf(Color(0xFF1E1B4B), Color(0xFF0F172A), Color(0xFF020617))))
    ) {
        Column(modifier = Modifier.fillMaxSize().padding(16.dp), horizontalAlignment = Alignment.CenterHorizontally) {
            Row(modifier = Modifier.fillMaxWidth().padding(top=24.dp), verticalAlignment = Alignment.CenterVertically) {
                Icon(Icons.Filled.Android, null, tint = Color(0xFF60A5FA))
                Spacer(Modifier.width(8.dp))
                Text("WinDroid V4", color = Color.White.copy(0.8f), fontSize = 14.sp)
            }
            Spacer(Modifier.weight(1f))
            Card(modifier = Modifier.fillMaxWidth(), shape = RoundedCornerShape(28.dp), colors = CardDefaults.cardColors(containerColor = Color(0xFF1E293B).copy(0.92f))) {
                Column(modifier = Modifier.padding(20.dp)) {
                    Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween, verticalAlignment = Alignment.CenterVertically) {
                        Text("Start", color = Color.White, fontSize = 26.sp, fontWeight = FontWeight.Bold)
                        Row {
                            Icon(Icons.Filled.Notifications, null, tint = Color.White, modifier = Modifier.padding(8.dp))
                            Icon(Icons.Filled.Menu, null, tint = Color.White, modifier = Modifier.padding(8.dp))
                        }
                    }
                    Text("Pinned", color = Color.White.copy(0.5f), fontSize = 12.sp, modifier = Modifier.padding(top=16.dp, bottom=12.dp))
                    LazyVerticalGrid(columns = GridCells.Fixed(3), modifier = Modifier.height(300.dp), userScrollEnabled = false) {
                        items(pinnedApps) { app ->
                            Column(modifier = Modifier.padding(6.dp).clip(RoundedCornerShape(12.dp)).clickable { Toast.makeText(context, "Opening ${app.name}", Toast.LENGTH_SHORT).show() }.padding(8.dp), horizontalAlignment = Alignment.CenterHorizontally) {
                                Box(modifier = Modifier.size(52.dp).clip(RoundedCornerShape(14.dp)).background(app.color.copy(0.18f)), contentAlignment = Alignment.Center) {
                                    Icon(app.icon, null, tint = app.color, modifier = Modifier.size(28.dp))
                                }
                                Spacer(Modifier.height(6.dp))
                                Text(app.name, color = Color.White, fontSize = 11.sp, maxLines = 1)
                            }
                        }
                    }
                    Text("Recommended", color = Color.White.copy(0.5f), fontSize = 12.sp, modifier = Modifier.padding(top=12.dp, bottom=8.dp))
                    Card(shape = RoundedCornerShape(16.dp), colors = CardDefaults.cardColors(containerColor = Color(0xFF0F172A))) {
                        Column(modifier = Modifier.padding(12.dp)) {
                            Row(verticalAlignment = Alignment.CenterVertically) {
                                Icon(Icons.Filled.Description, null, tint = Color.White.copy(0.8f), modifier = Modifier.size(24.dp))
                                Spacer(Modifier.width(12.dp))
                                Column {
                                    Text("Documents > report.pdf", color = Color.White, fontSize = 13.sp, fontWeight = FontWeight.Medium)
                                    Text("Today • 2.4 MB", color = Color.White.copy(0.5f), fontSize = 11.sp)
                                }
                            }
                            Spacer(Modifier.height(14.dp))
                            Row(verticalAlignment = Alignment.CenterVertically) {
                                Icon(Icons.Filled.Download, null, tint = Color.White.copy(0.8f), modifier = Modifier.size(24.dp))
                                Spacer(Modifier.width(12.dp))
                                Column {
                                    Text("Downloads > dxvk-setup.exe", color = Color.White, fontSize = 13.sp, fontWeight = FontWeight.Medium)
                                    Text("Yesterday • 15.2 MB", color = Color.White.copy(0.5f), fontSize = 11.sp)
                                }
                            }
                        }
                    }
                    Spacer(Modifier.height(16.dp))
                    Divider(color = Color.White.copy(0.08f))
                    Spacer(Modifier.height(12.dp))
                    Row(modifier = Modifier.fillMaxWidth(), verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.SpaceBetween) {
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            Box(modifier = Modifier.size(32.dp).clip(CircleShape).background(Color.Gray), contentAlignment = Alignment.Center) { Icon(Icons.Filled.Person, null, tint = Color.White) }
                            Spacer(Modifier.width(8.dp))
                            Column {
                                Text("User", color = Color.White, fontSize = 13.sp, fontWeight = FontWeight.Bold)
                                Text("Administrator", color = Color.White.copy(0.5f), fontSize = 11.sp)
                            }
                        }
                        Row {
                            Icon(Icons.Filled.PowerSettingsNew, null, tint = Color.White, modifier = Modifier.padding(8.dp))
                            Icon(Icons.Filled.Settings, null, tint = Color.White, modifier = Modifier.padding(8.dp))
                        }
                    }
                }
            }
            Spacer(Modifier.height(16.dp))
            Card(shape = RoundedCornerShape(32.dp), colors = CardDefaults.cardColors(containerColor = Color(0xFF1E293B).copy(0.95f))) {
                Row(modifier = Modifier.padding(horizontal = 22.dp, vertical = 12.dp), verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.spacedBy(20.dp)) {
                    Icon(Icons.Filled.Search, null, tint = Color.White.copy(0.7f))
                    Icon(Icons.Filled.Apps, null, tint = Color(0xFF60A5FA))
                    Icon(Icons.Filled.Folder, null, tint = Color(0xFFFBBF24))
                    Icon(Icons.Filled.WineBar, null, tint = Color(0xFFA78BFA))
                    Icon(Icons.Filled.ViewInAr, null, tint = Color(0xFFFB923C))
                    Icon(Icons.Filled.Language, null, tint = Color(0xFF22D3EE))
                }
            }
            Spacer(Modifier.height(28.dp))
        }
    }
}
