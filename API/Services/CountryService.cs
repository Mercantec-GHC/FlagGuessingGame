using System.Globalization;
using System.Text;
using System.Text.Json;
using ArduinoLandeQuizAPI.Models;
using Microsoft.Extensions.Caching.Memory;
using SkiaSharp;

namespace ArduinoLandeQuizAPI.Services
{
    public sealed class CountryService
    {
        private readonly IHttpClientFactory _httpClientFactory;
        private readonly IMemoryCache _memoryCache;

        private readonly List<Country> _previousCountries = new List<Country>();

        public CountryService(IHttpClientFactory httpClientFactory, IMemoryCache memoryCache)
        {
            _httpClientFactory = httpClientFactory;
            _memoryCache = memoryCache;
        }

        public async Task<RandomCountry> GetRandomCountryAsync(CancellationToken cancellationToken)
        {
            var countries = await GetCountries(cancellationToken);

            var selectedCountry = countries[Random.Shared.Next(countries.Count)];

            while (string.IsNullOrEmpty(selectedCountry.CCA3) || _previousCountries.Contains(selectedCountry))
            {
                selectedCountry = countries[Random.Shared.Next(countries.Count)];
            }

            _previousCountries.Add(selectedCountry);

            var choices = countries
                    .OrderBy(x => Guid.NewGuid())
                    .Where(x => x.CCA3 != selectedCountry.CCA3)
                    .Select(x => NormalizeString(x.Name.Common))
                    .Take(3)
                    .Append(NormalizeString(selectedCountry.Name.Common))
                    .OrderBy(x => Guid.NewGuid())
                    .ToList();

            return new RandomCountry
            {
                FlagCode = selectedCountry.CCA3,
                Choices = choices,
                CorrectChoiceIndex = choices.IndexOf(selectedCountry.Name.Common)
            };
        }

        public async Task<string> GetFlagUrlAsync(string code, CancellationToken cancellationToken)
        {
            var countries = await GetCountries(cancellationToken);

            return countries.FirstOrDefault(c => c.CCA3.Equals(code, StringComparison.OrdinalIgnoreCase))?.Flags.Png ?? string.Empty;
        }

        private async Task<List<Country>> GetCountries(CancellationToken cancellationToken)
        {
            if (!_memoryCache.TryGetValue("countries", out List<Country>? countries))
            {
                var httpClient = _httpClientFactory.CreateClient();

                var response = await httpClient.GetAsync("https://api-opgaver.mercantec.tech/api/Countries/simplified", cancellationToken);

                response.EnsureSuccessStatusCode();

                var json = await response.Content.ReadAsStringAsync(cancellationToken);

                countries = JsonSerializer.Deserialize<List<Country>>(json, new JsonSerializerOptions
                {
                    PropertyNameCaseInsensitive = true
                }) ?? new List<Country>();

                _memoryCache.Set("countries", countries, TimeSpan.FromDays(1));
            }

            return countries ?? new List<Country>();
        }

        private string NormalizeString(string input)
        {
            if (input.Contains('Ã') || input.Contains('Â'))
            {
                input = Encoding.UTF8.GetString(Encoding.Latin1.GetBytes(input));
            }

            input = input.Replace("Æ", "AE").Replace("æ", "ae").Replace("Ø", "O").Replace("ø", "o").Replace("ß", "ss");

            string normalized = input.Normalize(NormalizationForm.FormD);

            return new string(normalized.Where(c => CharUnicodeInfo.GetUnicodeCategory(c) != UnicodeCategory.NonSpacingMark && c <= 127).ToArray());
        }
    }
}
