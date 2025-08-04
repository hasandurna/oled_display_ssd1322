# Dikkat et

SSD1322 teknik dökümanında bulunan aşağıdaki tabloya göre t_cycle >= 300 ns gereksinimini baz alarak ~3MHz'lik bir SPI saat frekansı seçmeli. 300 ns altında çevrim süresi olursa veri geçerliliği ve komut setup/hold zamanları ihlal edilebilir. Bu yüzden 1/300ns ~= 3.33MHz yerine biraz da emniyet marjı koyarak 3MHz olarak ayarladık ilgili değeri daha da düşük bir değer seçebilirdik.

Bundan dolayı CubeMX ayarlarını yaparken doğru Prescaler değeri seçtiğinden emin ol

![Uploading 4 wire SPI.png…]()
