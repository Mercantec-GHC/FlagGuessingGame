using FlagGuessingGameAPI.Models;
using FlagGuessingGameAPI.Repositories;
using FlagGuessingGameAPI.Services;
using Microsoft.AspNetCore.Mvc;
using SkiaSharp;

namespace FlagGuessingGameAPI.Controllers
{
    [ApiController]
    [Route("api/[controller]/[action]")]
    public class QuizController : ControllerBase
    {
        private readonly IPlayerRepository _playerRepository;
        private readonly CountryService _countryService;

        public QuizController(IPlayerRepository playerRepository, CountryService countryService)
        {
            _playerRepository = playerRepository;
            _countryService = countryService;
        }

        [HttpGet]
        public async Task<ActionResult<List<Player>>> GetLeaderboard(CancellationToken cancellationToken)
        {
            return Ok(await _playerRepository.GetAllPlayersAsync(cancellationToken));
        }

        [HttpGet]
        public async Task<IActionResult> GetRandomCountry(CancellationToken cancellationToken = default)
        {
            try
            {
                var country = await _countryService.GetRandomCountryAsync(cancellationToken);

                return Ok(country);
            }
            catch (Exception)
            {
                return StatusCode(502, new { error = "Country data temporarily unavailable." });
            }
        }

        [HttpGet("{code}")]
        public async Task<IActionResult> GetFlag(string code, CancellationToken cancellationToken)
        {
            try
            {
                var httpClient = new HttpClient();

                var flagUrl = await _countryService.GetFlagUrlAsync(code, cancellationToken);

                var response = await httpClient.GetAsync(flagUrl, cancellationToken);

                if (!response.IsSuccessStatusCode)
                {
                    return NotFound(new { error = "Flag not found." });
                }

                byte[] data = await response.Content.ReadAsByteArrayAsync();

                using var source = SKBitmap.Decode(data);

                if (source == null)
                {
                    return BadRequest("Invalid image.");
                }

                const int width = 240;
                const int height = 240;

                using var bitmap = new SKBitmap(width, height);
                using var canvas = new SKCanvas(bitmap);

                canvas.Clear(SKColors.Black);

                float scale = Math.Min((float)width / source.Width, (float)height / source.Height);

                float scaledWidth = source.Width * scale;
                float scaledHeight = source.Height * scale;

                float x = (width - scaledWidth) / 2;
                float y = (height - scaledHeight) / 2;

                canvas.DrawBitmap(
                    source,
                    new SKRect(x, y, x + scaledWidth, y + scaledHeight),
                    new SKSamplingOptions(SKCubicResampler.Mitchell)
                );

                canvas.Flush();

                byte[] output = new byte[width * height * 2];
                int offset = 0;

                for (int py = 0; py < height; py++)
                {
                    for (int px = 0; px < width; px++)
                    {
                        SKColor pixel = bitmap.GetPixel(px, py);

                        ushort rgb565 = (ushort)(
                            ((pixel.Red & 0xF8) << 8) |
                            ((pixel.Green & 0xFC) << 3) |
                            (pixel.Blue >> 3)
                        );

                        output[offset++] = (byte)(rgb565 >> 8);
                        output[offset++] = (byte)rgb565;
                    }
                }

                return File(output, "application/octet-stream");
            }
            catch (Exception)
            {
                return StatusCode(502, new { error = "Flag data temporarily unavailable." });
            }
        }
    }
}
