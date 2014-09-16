var sharp = require('../index.js');


INPUT_PNG = './tests/fixtures/blackbug.png'
OUTPUT_1 = './output-1.jpg'
INPUT_JPG = './tests/fixtures/2569067123_aca715a2ee_o.jpg'
OUTPUT_2 = './output-2.jpg'

sharp(INPUT_PNG).metadata(function (error, metadata) {
  if (error) {
    throw error;
  }

  console.log('PNG metadata', metadata);
});

sharp(INPUT_PNG).background(255, 128, 0).flatten().rotate().toFile(OUTPUT_1, function (error, info) {
  if (error) {
    throw error;
  }

  console.log('\nPNG flatten & resize:', info);
});


sharp(INPUT_JPG).rotate().toFile(OUTPUT_2, function (error, info) {
  if (error) {
    throw error;
  }

  console.log('\nJPEG auto-orient:', info);
});

sharp(INPUT_JPG).background(255, 128, 0).flatten().toFile(OUTPUT_2, function (error, info) {
  if (error) {
    throw error;
  }

  console.log('\nJPEG flatten & resize:', info);
});
