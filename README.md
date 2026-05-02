# ArduThon2026
Repo untuk ArduThon 2026 berjudul SmartRoomIoT

### Cara Simulasi SmartRoom IoT di Wokwi (Step by Step)

#### Persiapan
- Pastikan Firebase URL sudah benar di kode (`https://smartroom-982da-default-rtdb.asia-southeast1.firebasedatabase.app/smartroom.json`).
- Buka project SmartRoom di **(`https://wokwi.com/projects/462890683615892481`)**.

---

#### Langkah Simulasi

**1. Menjalankan Simulator**
- Klik tombol **Run / Play** di Wokwi.
- Tunggu hingga ESP32 terhubung ke WiFi `Wokwi-GUEST`.
- OLED akan menampilkan "WiFi: OK" dan "Firebase: connected".

**2. Membuka Dashboard Web**
- Buka browser dan akses website kamu: `https://bromo.web.id/ArduThon2026`
- Masuk ke halaman SmartRoom.

**3. Mengatur Koneksi Firebase**
- Di kolom **FIREBASE DATABASE URL**, paste URL Firebase.
- Klik tombol **SAVE**.
- Pilih mode **FIREBASE LIVE**.

**4. Pengujian Sensor & Actuator**

- **Suhu & Kelembaban (DHT22)**  
  Klik sensor DHT22 pada diagram Wokwi → ubah nilai Temperature dan Humidity.

- **Cahaya (LDR)**  
  Putar **Potensiometer Kiri** (pin 34) untuk mengubah nilai Light (Lux).

- **CO₂ (Simulasi)**  
  Putar **Potensiometer Kanan** (pin 35) untuk mengubah nilai CO₂ (200–1200 ppm).

- **Lihat Hasil di OLED**  
  Perhatikan layar OLED menampilkan:
  - Nilai sensor real-time
  - Comfort Score
  - Status ruangan (COMFORTABLE, ACCEPTABLE, dll)

- **Lihat Reaksi Actuator**
  - **LED Merah** (pin 27) → Nyala saat kondisi CRITICAL
  - **LED Hijau** (pin 25) → Nyala saat kondisi COMFORTABLE
  - **LED Biru untuk Fan** (pin 14) → Nyala saat Fan aktif
  - **Buzzer** (pin 26) → Bunyi saat kondisi buruk

**5. Verifikasi di Dashboard Web**
- Nilai sensor, score, grafik history, dan status actuator harus update otomatis setiap 2 detik.
- Cek log reading dan ring score.

---

**Tips:**
- Semua perubahan di Wokwi akan terlihat bersamaan di **OLED** dan **Dashboard Web**.
- Buka Serial Monitor di Wokwi untuk melihat debug data.
