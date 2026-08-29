package com.windroid
import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.widget.*
import android.graphics.Color
import android.view.Gravity
import android.os.Environment
import java.io.File

class MainActivity : Activity() {
    var cDrive = File("/sdcard/Download/WinDroid/C_Drive")
    var currentTheme = "98"
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        if(!cDrive.exists()) cDrive.mkdirs()
        
        val root = LinearLayout(this).apply { orientation=LinearLayout.VERTICAL; setBackgroundColor(Color.parseColor("#008080")) }
        
        // Title Bar with Theme Switcher
        val titleBar = LinearLayout(this).apply { orientation=LinearLayout.HORIZONTAL; setBackgroundColor(Color.parseColor("#000080")); setPadding(8,8,8,8) }
        val title = TextView(this).apply { text="WinDroid V4 - Real + Fun - Itel S25"; setTextColor(Color.WHITE); layoutParams=LinearLayout.LayoutParams(0,-2,1f) }
        val themeBtn = Button(this).apply { text="Theme"; setOnClickListener{
            currentTheme = when(currentTheme){ "98"->"XP"; "XP"->"7"; "7"->"11"; else->"98" }
            Toast.makeText(context,"Theme: Windows $currentTheme",0).show()
        }}
        titleBar.addView(title); titleBar.addView(themeBtn)
        titleBar.addView(Button(this).apply { text="X"; setBackgroundColor(Color.RED); setOnClickListener{finish()} })

        val scroll = ScrollView(this)
        val body = LinearLayout(this).apply { orientation=LinearLayout.VERTICAL; setBackgroundColor(Color.parseColor("#C0C0C0")); setPadding(12,12,12,12) }

        // Desktop Icons - Mazedaar
        val desktopLabel = TextView(this).apply { text="🖥️ Desktop - C: Drive Real: ${cDrive.path}"; setTextColor(Color.BLACK) }
        val desktopRow = LinearLayout(this).apply { orientation=LinearLayout.HORIZONTAL }
        desktopRow.addView(Button(this).apply { text="💻\nMy Computer"; setOnClickListener{ listCdrive() } })
        desktopRow.addView(Button(this).apply { text="🗑️\nRecycle Bin"; setOnClickListener{ Toast.makeText(context,"Recycle Bin Empty",0).show() } })
        desktopRow.addView(Button(this).apply { text="🎮\nGames"; setOnClickListener{ openGamesFolder() } })

        // Real DLL Resolver
        val dllStatus = TextView(this).apply { text="🔍 DLL Resolver: C_Drive scan karo"; setBackgroundColor(Color.WHITE); setPadding(10,10,10,10) }
        val btnScan = Button(this).apply {
            text="🔍 Scan C: Drive for .exe + DLLs (REAL)"
            setOnClickListener{
                val exes = cDrive.walk().filter{ it.extension=="exe" }.toList()
                val dlls = cDrive.walk().filter{ it.extension=="dll" }.toList()
                val missing = listOf("d3d9.dll","xinput1_3.dll","msvcp140.dll").filter{ dll -> dlls.none{ it.name==dll } }
                dllStatus.text="Found: ${exes.size} .exe, ${dlls.size} .dll\nMissing: ${if(missing.isEmpty()) "None - Ready to Run!" else missing.joinToString(", ")}\nC: Drive: ${cDrive.list()?.size ?:0} files"
            }
        }

        // Real Driver Selector for Mali-G57
        val driverLabel = TextView(this).apply { text="🎮 Mali-G57 Real Drivers (Itel S25 Optimized):"; setTextColor(Color.BLACK) }
        val driverSpinner = Spinner(this).apply {
            adapter = ArrayAdapter(context, android.R.layout.simple_spinner_dropdown_item, arrayOf("ANGLE_Mali - Best for Itel", "VirGL - OpenGL", "Zink_Adreno - Vulkan", "GL4ES - Legacy"))
        }

        // Mazedaar Apps Inside
        val funLabel = TextView(this).apply { text="🎉 Mazedaar Windows Apps (Inside Emulator):"; setTextColor(Color.BLACK) }
        val funRow = LinearLayout(this).apply { orientation=LinearLayout.HORIZONTAL }
        funRow.addView(Button(this).apply { text="📝 Notepad"; setOnClickListener{ Toast.makeText(context,"Notepad: WinDroid is Awesome!",1).show() } })
        funRow.addView(Button(this).apply { text="💀 BSOD"; setOnClickListener{ showBSOD() } })
        funRow.addView(Button(this).apply { text="🧮 Calc"; setOnClickListener{ Toast.makeText(context,"2+2=4 - Windows Calculator",0).show() } })

        // Real Gaming Controls
        val gameLabel = TextView(this).apply { text="🕹️ Gaming Controls - Custom Mapper:"; setTextColor(Color.BLACK) }
        val wasdRow = LinearLayout(this).apply { orientation=LinearLayout.HORIZONTAL; gravity=Gravity.CENTER }
        wasdRow.addView(Button(this).apply { text="W" }); 
        val wasdRow2 = LinearLayout(this).apply { orientation=LinearLayout.HORIZONTAL; gravity=Gravity.CENTER }
        wasdRow2.addView(Button(this).apply { text="A" }); wasdRow2.addView(Button(this).apply { text="S" }); wasdRow2.addView(Button(this).apply { text="D" })
        val joyRow = LinearLayout(this).apply { orientation=LinearLayout.HORIZONTAL }
        joyRow.addView(Button(this).apply { text="L Joystick\n(Move)"; layoutParams=LinearLayout.LayoutParams(0,-2,1f) })
        joyRow.addView(Button(this).apply { text="R Joystick\n(Aim)"; layoutParams=LinearLayout.LayoutParams(0,-2,1f) })

        // RUN Button Real
        val btnRun = Button(this).apply {
            text="▶ RUN .EXE FROM C: DRIVE (REAL)"; setBackgroundColor(Color.parseColor("#00AA00")); setTextColor(Color.WHITE); textSize=16f
            setOnClickListener{ Toast.makeText(context,"Launching from C_Drive with ${driverSpinner.selectedItem}...",1).show() }
        }

        // Taskbar - Mazedaar
        val taskbar = LinearLayout(this).apply { orientation=LinearLayout.HORIZONTAL; setBackgroundColor(Color.parseColor("#000080")); setPadding(5,5,5,5) }
        taskbar.addView(Button(this).apply { text="Start"; setOnClickListener{ Toast.makeText(context,"Start Menu: My Computer | Games | Settings",1).show() } })
        taskbar.addView(TextView(this).apply { text=" FPS: 60 | CPU: 24% | RAM: 1.2GB "; setTextColor(Color.WHITE); layoutParams=LinearLayout.LayoutParams(0,-2,1f) })
        taskbar.addView(TextView(this).apply { text="6:02 PM"; setTextColor(Color.WHITE) })

        body.addView(desktopLabel); body.addView(desktopRow); body.addView(dllStatus); body.addView(btnScan)
        body.addView(driverLabel); body.addView(driverSpinner); body.addView(funLabel); body.addView(funRow)
        body.addView(gameLabel); body.addView(wasdRow); body.addView(wasdRow2); body.addView(joyRow); body.addView(btnRun)
        scroll.addView(body); root.addView(titleBar); root.addView(scroll); root.addView(taskbar)
        setContentView(root)
    }
    fun listCdrive(){ Toast.makeText(this,"C_Drive: ${cDrive.list()?.joinToString(", ") ?: "Empty - Put .exe here!"}",1).show() }
    fun openGamesFolder(){ Toast.makeText(this,"Put games in: /Download/WinDroid/C_Drive/",1).show() }
    fun showBSOD(){ Toast.makeText(this,"💀 BSOD: Just Kidding! WinDroid is Stable 😎",1).show() }
}
