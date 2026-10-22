using ArduinoLandeQuizAPI.Models;
using ArduinoLandeQuizAPI.Repositories;
using ArduinoLandeQuizAPI.Services;
using Microsoft.AspNetCore.Mvc;

namespace ArduinoLandeQuizAPI.Controllers
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
                
                var imageBytes = await response.Content.ReadAsByteArrayAsync(cancellationToken);

                return File(imageBytes, "image/png");
            }
            catch (Exception)
            {
                return StatusCode(502, new { error = "Flag data temporarily unavailable." });
            }
        }
    }
}
