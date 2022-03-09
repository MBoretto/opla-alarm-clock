#ifndef DATE_TIME
#define DATE_TIME

class DateTime {

public:
  DateTime(uint8_t hours, uint8_t minutes, uint8_t seconds = 0):
    m_time_seconds(hours * 60 * 60 + minutes * 60 + seconds) {
  }

  DateTime(uint time_seconds):
    m_time_seconds(time_seconds) {
  }

  uint8_t getHours() const {
      uint raw_minutes = m_time_seconds / 60;
      return raw_minutes / 60;
  }
  uint8_t getMinutes() const {
      uint raw_minutes = m_time_seconds / 60;
      return raw_minutes % 60;
  }

  inline DateTime operator - (const uint& offset_seconds) const {
    uint new_time_seconds = 0;
    if (m_time_seconds < offset_seconds) {
      new_time_seconds = 60 * 60 * 24 + m_time_seconds - offset_seconds;
    } else {
       new_time_seconds = m_time_seconds - offset_seconds;
    }

    return DateTime(new_time_seconds);
  }

  inline DateTime operator + (const uint& offset_seconds) const {
    uint new_time_seconds = m_time_seconds + offset_seconds;
    if (new_time_seconds > 60 * 60 * 24) {
      new_time_seconds -= 60 * 60 * 24;
    }

    return DateTime(new_time_seconds);
  }

  inline bool operator > (const DateTime& adate_time) const {
    if (adate_time.m_time_seconds > m_time_seconds) {
      return false;
    }
    return true;
  }

  inline bool operator < (const DateTime& adate_time) const {
    if (m_time_seconds < adate_time.m_time_seconds) {
      return true;
    }
    return false;
  }

private:
  uint m_time_seconds; //number of seconds from 00:00:00

};

#endif
