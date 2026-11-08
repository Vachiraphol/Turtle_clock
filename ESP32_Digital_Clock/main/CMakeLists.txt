# ระบุไฟล์ source (.c) ทั้งหมดที่มีในโฟลเดอร์ main 
# (ถ้ามีไฟล์ .c อื่นๆ ให้เติมชื่อลงไปในวงเล็บด้วย)
idf_component_register(SRCS "main.c" "time_settings.c" "timezone.c"
                       INCLUDE_DIRS ".")