#line 1 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/buffer.h"

template<size_t N>
struct Buffer {

public:
  uint8_t* data_ptr;
  static constexpr size_t capacity = N;
  size_t head = 0;
  size_t tail = 0;
  bool full = false;

  bool is_full() const { return full; }
  bool is_empty() const { return !full && (head == tail); }


  size_t size() const {
    if(full) return capacity;
    if(head >= tail) return head - tail;
    return capacity + head - tail;
  }

  void put(uint8_t byte) {
    data_ptr[head] = byte;
      advance();
  }

  int get(uint8_t* byte) {
    if(is_empty()) return -1;
    *byte = data_ptr[tail];
    retreat();
    return 0;
  }

  int read(uint8_t* buf,size_t len){
    len = min(size(),len);
    memcpy(buf,data_ptr+tail,len);
    retreat(len);
    return len;
  };

  int peek(uint8_t* buf,size_t len){
    len = min(size(),len);
    memcpy(buf,data_ptr+tail,len);
    return len;
  };

  void reset(){
    head = 0;
    tail = 0;
    full = false;
  };

#ifdef ESP32
  void put_serial(HardwareSerial& stream){
    size_t len = stream.available();
    if (len == 0)
      return;

    // Limit to buffer free space if you DON'T want overwrite:
    // len = std::min(len, capacity - size());

    // 1) If buffer isn't full, and fits without overflow → single read
    if (!full && head + len <= capacity) {
      stream.readBytes(&data_ptr[head], len);
      head += len;
      if (head == capacity) head = 0;
      full = (head == tail);
      return;
    }

    // 2) Otherwise handle wrap OR overwrite-oldest case

    Serial.println("OVF");
    // First chunk: until end
    size_t first = std::min(len, capacity - head);
    stream.readBytes(&data_ptr[head], first);

    // Second chunk: from start of buffer
    size_t second = len - first;
    if (second > 0) {
      stream.readBytes(&data_ptr[0], second);
    }

    // Advance head
    head = (head + len) % capacity;

    // If we overflowed while buffer was full, move tail forward too
    if (full) {
      tail = (tail + len) % capacity;
    }

    full = (head == tail);
  };
#endif

  void advance(size_t n = 1) {
    if(n == 0) return;
    if(full) { // overwrite oldest
      tail = (tail + n) % capacity;
    }
    head = (head + n) % capacity;
    full = (head == tail);
  }

  void retreat(size_t n = 1) {
    if(n == 0) return;
    full = false;
    tail = (tail + n) % capacity;
  }
};


template<size_t N>
struct StackBuffer : public Buffer<N> {
  uint8_t data [N];
  StackBuffer(){
    this->data_ptr = data;
  }
};


template<size_t N>
struct RAMBuffer : public Buffer<N> {
  uint8_t* data;

  RAMBuffer(){
    uint8_t* data = (uint8_t*)malloc(N);
    this->data_ptr = data;
    if (data == NULL) {
      Serial.println("FAILED TO ALLOCATE RAMBuffer");
      // handle error
    }
  };
  ~RAMBuffer(){
    free(data);
  }

};

template<size_t N>
struct DMABuffer : public Buffer<N> {
  uint8_t* data;
  DMABuffer(){
    uint8_t* data = (uint8_t*)heap_caps_malloc(N, MALLOC_CAP_DMA);
    if (data == NULL) {
      Serial.println("FAILED TO ALLOCATE DMA");
      // handle error
    }
  };
  ~DMABuffer(){
    heap_caps_free(data);
  }

};
